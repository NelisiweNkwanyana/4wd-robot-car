// CommandInput.h — single-character command protocol, read from BOTH the
// HC-05 (SoftwareSerial D2/D3) and the USB Serial Monitor, so every mode can
// be bench-tested from the laptop before the phone is involved.
//
// Drive (manual mode)   F fwd  B back  L spin-left  R spin-right
//                       G fwd-left  I fwd-right  H back-left  J back-right
//                       S stop (in manual: stop motors; elsewhere: stop -> IDLE)
// Speed                 0..9 = 0..90 %, q = 100 %
// Modes                 T line Tracking   O Obstacle   M Manual   K joystick (stick)
// Safety / info         E or ! = emergency stop -> IDLE from any mode
//                       D (some apps' "all off") = S      ? = status report
// Anything else (W w U u V v X x, CR, LF) is ignored, so light/horn buttons
// on RC-car apps can't change modes by accident.
//
// Drive keys are "hold to drive": the app must keep resending the key
// (most RC-car apps do while a button is held). If no drive key arrives for
// BT_TIMEOUT_MS the manual command decays to STOP — this is the
// communication-loss failsafe.
#pragma once
#include "Types.h"

class CommandInput {
public:
  enum class Request : uint8_t { NONE, MODE_LINE, MODE_OBSTACLE, MODE_MANUAL,
                                 MODE_JOYSTICK, STOP, ESTOP, STATUS };

  void begin();
  void update(unsigned long now);           // drains both serial ports
  void handleChar(char c, unsigned long now);

  Request takeRequest();                    // returns pending request and clears it
  bool takeDriveKeyFlag();                  // true if a drive key arrived since last call
  DriveCommand manualDrive(unsigned long now) const;  // STOP once timed out
  bool linkTimedOut(unsigned long now) const;
  void clearDrive() { driveKey = 'S'; }
  uint8_t speedPct() const { return speed; }
  char lastKey() const { return driveKey; }

  void reply(const __FlashStringHelper* msg);   // to USB and Bluetooth
  Print& bluetooth();

private:
  void post(Request r);
  Request pending = Request::NONE;
  bool driveFlag = false;
  char driveKey = 'S';
  unsigned long lastDriveMs = 0;
  uint8_t speed = 60;
};
