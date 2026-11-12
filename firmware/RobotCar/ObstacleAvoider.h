// ObstacleAvoider.h — drive forward, stop at OBSTACLE_STOP_CM, back off,
// look left and right with the servo, turn toward the clearer side, re-check.
//
//  CRUISE ──close──► HALT ─► REVERSE ─► LOOK_LEFT ─► LOOK_RIGHT ─► TURN ─► CHECK
//    ▲                                                                       │
//    └──────────────────────── front clear ◄──────────────────────────────────┘
//  CHECK still blocked -> HALT again; after AVOID_MAX_RETRIES -> STUCK (fault).
#pragma once
#include "Types.h"
#include "Ultrasonic.h"
#include "Scanner.h"

class ObstacleAvoider {
public:
  enum class State : uint8_t { CRUISE, HALT, REVERSE, LOOK_LEFT, LOOK_RIGHT, TURN, CHECK, STUCK };

  void reset(unsigned long now, Scanner& sc);
  DriveCommand update(Ultrasonic& us, Scanner& sc, unsigned long now);
  State state() const { return st; }
  bool stuck() const { return st == State::STUCK; }

private:
  void enter(State s, unsigned long now) { st = s; since = now; }
  bool freshReadingAfterSettle(const Ultrasonic& us, Scanner& sc, unsigned long now);

  State st = State::CRUISE;
  unsigned long since = 0;
  uint32_t samplesAtSettle = 0;
  bool waitingForSample = false;
  uint16_t leftCm = 0, rightCm = 0;
  int8_t turnDir = -1;         // -1 left, +1 right
  uint16_t turnMs = AVOID_TURN_MS;
  uint8_t retries = 0;
};
