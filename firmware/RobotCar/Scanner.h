// Scanner.h — SG90 servo that pans the HC-SR04. Only moved while the car
// is stopped (servo current spikes + motor current = brown-out risk), and
// detached when parked at centre so it stops drawing holding current.
#pragma once
#include <Arduino.h>

class Scanner {
public:
  void begin();
  void lookTo(uint8_t deg, unsigned long now);   // starts a move (non-blocking)
  bool settled(unsigned long now) const;          // true once SERVO_SETTLE_MS has passed
  void park(unsigned long now) { lookTo(centerDeg(), now); parking = true; }
  void update(unsigned long now);                 // detaches after parking
  uint8_t angle() const { return current; }
  static uint8_t centerDeg();
private:
  uint8_t current = 90;
  unsigned long movedAt = 0;
  bool attached = false;
  bool parking = false;
};
