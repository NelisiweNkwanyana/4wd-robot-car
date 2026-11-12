#pragma once
#include "Arduino.h"

namespace sim { extern int servoAngle; extern bool servoAttached; extern int servoAttachCount; }

class Servo {
public:
  void attach(int) { sim::servoAttached = true; sim::servoAttachCount++; }
  void detach() { sim::servoAttached = false; }
  void write(int a) { sim::servoAngle = a; }
};
