// Motors.h — L298N driver for a skid-steer car (left pair on channel A,
// right pair on channel B). Speeds ramp toward the target so direction
// reversals always pass through zero; stopNow() skips the ramp.
#pragma once
#include "Types.h"

class Motors {
public:
  void begin();
  void setTarget(const DriveCommand& cmd);   // percent, applied by update()
  void stopNow();                            // emergency: outputs off this instant
  void update(unsigned long now);
  bool isStopped() const { return curLeft == 0 && curRight == 0; }
  int16_t pwmLeft()  const { return curLeft; }   // signed PWM actually applied
  int16_t pwmRight() const { return curRight; }

  static int16_t percentToPwm(int8_t pct);   // public for tests

private:
  void apply();
  static int16_t stepToward(int16_t cur, int16_t target);
  static void writeSide(uint8_t en, uint8_t inA, uint8_t inB, int16_t pwm);

  int16_t tgtLeft = 0, tgtRight = 0;
  int16_t curLeft = 0, curRight = 0;
  unsigned long lastRamp = 0;
};
