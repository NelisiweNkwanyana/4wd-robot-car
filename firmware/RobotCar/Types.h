// Types.h — shared types. Behaviour and input modules only ever produce a
// DriveCommand; only ModeManager hands one to Motors.
#pragma once
#include <Arduino.h>

// Wheel speeds in percent, -100 (full reverse) .. +100 (full forward).
struct DriveCommand {
  int8_t left;
  int8_t right;
};

inline DriveCommand makeDrive(int left, int right) {
  DriveCommand c;
  c.left  = (int8_t)constrain(left,  -100, 100);
  c.right = (int8_t)constrain(right, -100, 100);
  return c;
}

const DriveCommand DRIVE_STOP = {0, 0};

// Arcade mix: throttle/steering (-100..100) -> left/right wheel percent.
// Shared by the Bluetooth keys and the joystick so both feel the same.
inline DriveCommand arcadeMix(int throttle, int steering) {
  int left  = throttle + steering;
  int right = throttle - steering;
  int biggest = max(abs(left), abs(right));
  if (biggest > 100) {               // scale down, keep the ratio
    left  = (long)left  * 100 / biggest;
    right = (long)right * 100 / biggest;
  }
  return makeDrive(left, right);
}

enum class Mode : uint8_t { IDLE, LINE_FOLLOW, OBSTACLE, BT_MANUAL, JOYSTICK, FAULT };

enum class Fault : uint8_t {
  NONE,
  LOW_BATTERY,
  LINE_LOST,
  LINE_LIFTED,       // all three sensors black for too long
  ULTRASONIC,        // HC-SR04 not responding / stuck
  JOYSTICK_INVALID,
  OBSTACLE_STUCK,    // boxed in, avoidance retries exhausted
};

const __FlashStringHelper* modeName(Mode m);
const __FlashStringHelper* faultName(Fault f);
