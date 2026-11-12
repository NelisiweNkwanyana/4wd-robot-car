// =====================================================================
//  4WD Robot Car — Arduino UNO firmware
//  Modes: line following, obstacle avoidance, Bluetooth manual, joystick
//
//  Open this folder in Arduino IDE 2, board "Arduino Uno", upload.
//  Serial Monitor at 115200 baud accepts the same commands as the phone:
//    T line  O obstacle  M manual  K joystick  S stop  E emergency  ? status
//    F B L R G I H J drive (hold-to-drive: resend within 600 ms)   0-9 q speed
//
//  All pins and thresholds: Config.h. Wiring: docs/wiring.md.
//  Status: written and logic-tested on a host simulator only — NOT yet
//  tested on the physical robot. See docs/testing-guide.md.
// =====================================================================
#include "Config.h"
#include "Types.h"
#include "ModeManager.h"
#include "StatusLed.h"

Robot bot;
ModeManager modes(bot);
StatusLed led;
unsigned long lastDebug = 0;

void setup() {
  // Motors first: make sure the L298N is off before anything else happens.
  bot.motors.begin();
  Serial.begin(DEBUG_BAUD);
  bot.input.begin();
  bot.line.begin();
  bot.sonar.begin();
  bot.scanner.begin();
  bot.joystick.begin();     // calibrates centre: leave the stick alone at power-on
  bot.battery.begin();
  led.begin();

  Serial.println(F("\n4WD Robot Car ready. Commands: T O M K S E ? | F B L R | 0-9 q"));
  if (JOYSTICK_FITTED && !bot.joystick.valid())
    Serial.println(F("WARN joystick centre calibration failed - was the stick moved?"));
  modes.begin(millis());
}

void loop() {
  unsigned long now = millis();

  // 1. Inputs and sensors (each limits its own rate)
  bot.input.update(now);
  bot.line.read();
  bot.sonar.update(now);     // blocks <= ~9 ms every 60 ms
  bot.joystick.update(now);
  bot.battery.update(now);
  bot.scanner.update(now);

  // 2. Decide (the only place a motor command is chosen)
  modes.update(now);

  // 3. Act
  bot.motors.update(now);
  led.update(modes.mode(), now);

  // 4. Debug line
  if (DEBUG_ENABLED && now - lastDebug >= DEBUG_INTERVAL_MS) {
    lastDebug = now;
    Serial.print(F("t=")); Serial.print(now); Serial.print(' ');
    modes.printStatus(Serial);
  }
}
