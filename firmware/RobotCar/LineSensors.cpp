#include "LineSensors.h"
#include "Config.h"

void LineSensors::begin() {
  pinMode(PIN_LINE_LEFT, INPUT);
  pinMode(PIN_LINE_CENTER, INPUT);
  pinMode(PIN_LINE_RIGHT, INPUT);
}

bool LineSensors::onLine(uint8_t pin) {
  // Most TCRT5000 modules: dark surface reflects little IR -> DO goes HIGH.
  // Check yours with firmware-tests/02_line_sensor_test and set LINE_ACTIVE_LOW.
  bool high = digitalRead(pin) == HIGH;
  return LINE_ACTIVE_LOW ? !high : high;
}

uint8_t LineSensors::read() {
  lastMask = (onLine(PIN_LINE_LEFT)   ? LineMask::LEFT   : 0) |
             (onLine(PIN_LINE_CENTER) ? LineMask::CENTER : 0) |
             (onLine(PIN_LINE_RIGHT)  ? LineMask::RIGHT  : 0);
  return lastMask;
}
