#include "CommandInput.h"
#include "Config.h"
#include <SoftwareSerial.h>

static SoftwareSerial btSerial(PIN_BT_RX, PIN_BT_TX);

void CommandInput::begin() {
  btSerial.begin(BT_BAUD);
  speed = DEFAULT_SPEED_PCT;
}

Print& CommandInput::bluetooth() { return btSerial; }

void CommandInput::reply(const __FlashStringHelper* msg) {
  Serial.println(msg);
  btSerial.println(msg);
}

void CommandInput::update(unsigned long now) {
  // Bounded loops: at 9600 baud at most ~1 byte/ms arrives, buffers are 64 bytes.
  uint8_t n = 0;
  while (btSerial.available() && n++ < 32) handleChar((char)btSerial.read(), now);
  n = 0;
  while (Serial.available() && n++ < 32)   handleChar((char)Serial.read(), now);
}

void CommandInput::post(Request r) {
  // Stops always win over anything already queued this loop.
  if (pending == Request::ESTOP) return;
  if (pending == Request::STOP && r != Request::ESTOP) return;
  pending = r;
}

void CommandInput::handleChar(char c, unsigned long now) {
  switch (c) {
    case 'F': case 'B': case 'L': case 'R':
    case 'G': case 'I': case 'H': case 'J':
      driveKey = c; lastDriveMs = now; driveFlag = true; return;
    case 'S': case 'D':
      driveKey = 'S'; lastDriveMs = now; post(Request::STOP); return;
    case 'E': case '!':
      driveKey = 'S'; post(Request::ESTOP); return;
    case 'q': speed = 100; return;
    case 'T': post(Request::MODE_LINE); return;
    case 'O': post(Request::MODE_OBSTACLE); return;
    case 'M': post(Request::MODE_MANUAL); return;
    case 'K': post(Request::MODE_JOYSTICK); return;
    case '?': post(Request::STATUS); return;
    default:
      if (c >= '0' && c <= '9') speed = (c - '0') * 10;
      return;   // everything else ignored on purpose
  }
}

CommandInput::Request CommandInput::takeRequest() {
  Request r = pending;
  pending = Request::NONE;
  return r;
}

bool CommandInput::takeDriveKeyFlag() {
  bool f = driveFlag;
  driveFlag = false;
  return f;
}

bool CommandInput::linkTimedOut(unsigned long now) const {
  return now - lastDriveMs > BT_TIMEOUT_MS;
}

DriveCommand CommandInput::manualDrive(unsigned long now) const {
  if (driveKey == 'S' || linkTimedOut(now)) return DRIVE_STOP;
  int s = speed, h = speed / 2;
  switch (driveKey) {
    case 'F': return makeDrive(s, s);
    case 'B': return makeDrive(-s, -s);
    case 'L': return makeDrive(-s, s);     // spin in place
    case 'R': return makeDrive(s, -s);
    case 'G': return makeDrive(h, s);      // curve: left side slower
    case 'I': return makeDrive(s, h);
    case 'H': return makeDrive(-h, -s);
    case 'J': return makeDrive(-s, -h);
    default:  return DRIVE_STOP;
  }
}
