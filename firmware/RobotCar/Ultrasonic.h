// Ultrasonic.h — HC-SR04 with validation and a 3-sample median filter.
//
// Each measurement blocks for at most US_RISE_TIMEOUT_US + US_TIMEOUT_US
// (~9 ms) and runs every US_INTERVAL_MS. Readings are classified:
//   valid         2..US_MAX_CM
//   out of range  echo started but no return in time -> reported as US_MAX_CM (clear)
//   invalid       echo already high before trigger, never started, or < US_MIN_CM
// US_MAX_BAD_READS invalid readings in a row -> hasFault() (wiring/sensor problem).
#pragma once
#include "Config.h"

class Ultrasonic {
public:
  void begin();
  void update(unsigned long now);
  uint16_t distanceCm() const { return filtered; }  // median of last 3 good readings
  uint16_t latestCm() const { return latest; }      // most recent good reading
  bool hasReading() const { return goodCount > 0; }
  bool hasFault() const { return badStreak >= US_MAX_BAD_READS; }
  uint32_t sampleCount() const { return samples; }  // increments on every good reading
  void resetFault() { badStreak = 0; }
  // Forget old readings (call after the servo points somewhere new).
  void resetFilter() { goodCount = 0; }

  // Result codes for measureOnce(); public so tests can check classification.
  static const int16_t INVALID = -1;
  int16_t measureOnce();

private:
  uint16_t history[3] = {0, 0, 0};
  uint8_t  histIdx = 0;
  uint8_t  goodCount = 0;
  uint16_t filtered = 0;
  uint16_t latest = 0;
  uint8_t  badStreak = 0;
  uint32_t samples = 0;
  unsigned long lastPing = 0;
};
