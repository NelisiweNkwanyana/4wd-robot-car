#include "StatusLed.h"
#include "Config.h"

void StatusLed::begin() { pinMode(PIN_STATUS_LED, OUTPUT); }

void StatusLed::update(Mode m, unsigned long now) {
  bool on;
  if (m == Mode::FAULT) {
    on = (now / 80) % 2;                         // ~6 Hz flicker
  } else {
    uint8_t blinks = (uint8_t)m + 1;             // IDLE=1 ... JOYSTICK=5
    unsigned long t = now % 2000;
    uint8_t slot = t / 200;                      // 200 ms slots: on 100, off 100
    on = slot < blinks && (t % 200) < 100;
  }
  digitalWrite(PIN_STATUS_LED, on ? HIGH : LOW);
}
