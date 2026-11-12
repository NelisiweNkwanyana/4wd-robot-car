// Battery.h — 2S pack voltage on A5 through a 10k/10k divider.
// Filtered (EMA) and debounced: the voltage must stay below the cut-off for
// BATTERY_LOW_HOLD_MS before low() latches, so brief motor-start sag is
// ignored. Clears only above BATTERY_RESUME_V (hysteresis).
// Below BATTERY_PRESENT_V the board is assumed to be on USB power only:
// that's reported, not treated as a fault, so bench testing still works.
#pragma once
#include <Arduino.h>

class Battery {
public:
  void begin();
  void update(unsigned long now);
  void feedVolts(float v, unsigned long now);  // filter + latch logic (testable)
  float volts() const { return filtered; }
  bool present() const { return filtered >= presentV(); }
  bool low() const { return isLow; }
private:
  static float presentV();
  float filtered = 0;
  bool primed = false, isLow = false, belowCut = false;
  unsigned long belowSince = 0, lastRead = 0;
};
