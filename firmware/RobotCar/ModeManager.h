// ModeManager.h — the ONLY module that commands the motors.
//
// Each loop: take requests from CommandInput -> check faults -> ask the
// active behaviour for a DriveCommand -> pass it through the safety filter
// -> Motors::setTarget(). Rules:
//   * every mode change ramps to STOP and waits MODE_SETTLE_MS first
//   * faults cut the motors instantly (no ramp) and latch until S / E
//   * E (emergency stop) works from every mode, including during transitions
#pragma once
#include "Types.h"
#include "Motors.h"
#include "LineSensors.h"
#include "Ultrasonic.h"
#include "Scanner.h"
#include "LineFollower.h"
#include "ObstacleAvoider.h"
#include "CommandInput.h"
#include "JoystickInput.h"
#include "Battery.h"

struct Robot {
  Motors motors;
  LineSensors line;
  Ultrasonic sonar;
  Scanner scanner;
  LineFollower follower;
  ObstacleAvoider avoider;
  CommandInput input;
  JoystickInput joystick;
  Battery battery;
};

class ModeManager {
public:
  explicit ModeManager(Robot& r) : bot(r) {}
  void begin(unsigned long now);
  void update(unsigned long now);
  void requestMode(Mode m, unsigned long now);

  Mode mode() const { return current; }
  Mode pendingMode() const { return pending; }
  bool inTransition() const { return transitioning; }
  Fault fault() const { return activeFault; }
  DriveCommand lastCommand() const { return lastCmd; }
  bool linePausedForObstacle() const { return linePaused; }
  void printStatus(Print& out) const;

private:
  void handleRequests(unsigned long now);
  void enterFault(Fault f, unsigned long now);
  void onEnter(Mode m, unsigned long now);
  DriveCommand behaviour(unsigned long now);
  DriveCommand safetyFilter(DriveCommand c) const;
  bool obstacleAhead() const;
  void announce(unsigned long now);

  Robot& bot;
  Mode current = Mode::IDLE, pending = Mode::IDLE;
  bool transitioning = false;
  unsigned long transitionAt = 0;
  Fault activeFault = Fault::NONE;
  DriveCommand lastCmd = DRIVE_STOP;
  bool linePaused = false;
  unsigned long lineClearSince = 0;
};
