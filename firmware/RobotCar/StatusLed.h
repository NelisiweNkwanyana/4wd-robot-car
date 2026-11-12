// StatusLed.h — on-board LED (D13) shows the mode as a blink count every 2 s:
//   IDLE 1   LINE 2   OBSTACLE 3   MANUAL 4   JOYSTICK 5   FAULT = fast flicker
#pragma once
#include "Types.h"

class StatusLed {
public:
  void begin();
  void update(Mode m, unsigned long now);
};
