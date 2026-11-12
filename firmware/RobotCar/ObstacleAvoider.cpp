#include "ObstacleAvoider.h"
#include "Config.h"

void ObstacleAvoider::reset(unsigned long now, Scanner& sc) {
  retries = 0;
  waitingForSample = false;
  sc.park(now);
  enter(State::CRUISE, now);
}

// True once the servo has settled AND the sensor has produced a reading
// that was taken after settling (so it measures the new direction).
bool ObstacleAvoider::freshReadingAfterSettle(const Ultrasonic& us, Scanner& sc,
                                              unsigned long now) {
  if (!sc.settled(now)) { waitingForSample = false; return false; }
  if (!waitingForSample) { waitingForSample = true; samplesAtSettle = us.sampleCount(); }
  return us.sampleCount() > samplesAtSettle;
}

DriveCommand ObstacleAvoider::update(Ultrasonic& us, Scanner& sc, unsigned long now) {
  unsigned long t = now - since;

  switch (st) {
    case State::CRUISE: {
      if (!us.hasReading()) return DRIVE_STOP;              // wait for first reading
      uint16_t d = us.distanceCm();
      if (d <= OBSTACLE_STOP_CM) { enter(State::HALT, now); return DRIVE_STOP; }
      int pct = d <= OBSTACLE_SLOW_CM ? AVOID_SLOW_PCT : AVOID_CRUISE_PCT;
      return makeDrive(pct, pct);
    }

    case State::HALT:
      if (t >= AVOID_STOP_MS) enter(State::REVERSE, now);
      return DRIVE_STOP;

    case State::REVERSE:
      if (t < AVOID_REVERSE_MS) return makeDrive(-AVOID_REVERSE_PCT, -AVOID_REVERSE_PCT);
      if (SERVO_FITTED) {
        sc.lookTo(SERVO_LEFT_DEG, now);
        waitingForSample = false;
        enter(State::LOOK_LEFT, now);
      } else {                                              // no servo: always try left
        turnDir = -1; turnMs = AVOID_TURN_MS;
        enter(State::TURN, now);
      }
      return DRIVE_STOP;

    case State::LOOK_LEFT:
      if (freshReadingAfterSettle(us, sc, now)) {
        leftCm = us.latestCm();
        sc.lookTo(SERVO_RIGHT_DEG, now);
        waitingForSample = false;
        enter(State::LOOK_RIGHT, now);
      }
      return DRIVE_STOP;

    case State::LOOK_RIGHT:
      if (freshReadingAfterSettle(us, sc, now)) {
        rightCm = us.latestCm();
        sc.park(now);
        if (leftCm <= OBSTACLE_STOP_CM && rightCm <= OBSTACLE_STOP_CM) {
          turnDir = -1; turnMs = AVOID_TURN_MS * 2;         // both blocked: about-face
        } else {
          turnDir = leftCm >= rightCm ? -1 : +1;
          turnMs = AVOID_TURN_MS;
        }
        enter(State::TURN, now);
      }
      return DRIVE_STOP;

    case State::TURN:
      if (t < turnMs)
        return turnDir < 0 ? makeDrive(-AVOID_TURN_PCT, AVOID_TURN_PCT)
                           : makeDrive(AVOID_TURN_PCT, -AVOID_TURN_PCT);
      waitingForSample = false;
      enter(State::CHECK, now);
      return DRIVE_STOP;

    case State::CHECK:
      // Servo is back at centre (parked during LOOK_RIGHT); take a fresh reading.
      if (freshReadingAfterSettle(us, sc, now)) {
        if (us.latestCm() > OBSTACLE_STOP_CM) {
          retries = 0;
          us.resetFilter();// drop left/right scan values
          enter(State::CRUISE, now);
        } else if (++retries >= AVOID_MAX_RETRIES) {
          enter(State::STUCK, now);
        } else {
          enter(State::HALT, now);
        }
      }
      return DRIVE_STOP;

    case State::STUCK:
    default:
      return DRIVE_STOP;
  }
}
