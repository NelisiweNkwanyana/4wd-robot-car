// LineSensors.h — three digital IR modules (TCRT5000-type, DO pin).
// read() returns a 3-bit mask: bit2 = LEFT, bit1 = CENTRE, bit0 = RIGHT,
// where 1 means "this sensor sees the dark line".
#pragma once
#include <Arduino.h>

namespace LineMask {
  const uint8_t RIGHT  = 0b001;
  const uint8_t CENTER = 0b010;
  const uint8_t LEFT   = 0b100;
}

class LineSensors {
public:
  void begin();
  uint8_t read();
  uint8_t last() const { return lastMask; }
private:
  static bool onLine(uint8_t pin);
  uint8_t lastMask = 0;
};
