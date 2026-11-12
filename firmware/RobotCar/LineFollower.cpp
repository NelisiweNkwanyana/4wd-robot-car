#include "LineFollower.h"
#include "LineSensors.h"
#include "Config.h"

void LineFollower::reset(unsigned long now) {
  last = DRIVE_STOP;
  lastSide = 0;
  lineSeenAt = now;
  allBlack = false;
  st = Status::FOLLOWING;
}

DriveCommand LineFollower::update(uint8_t mask, unsigned long now) {
  if (st == Status::LOST || st == Status::LIFTED) return DRIVE_STOP;  // latched until reset

  // Three sensors on black for a long time: robot lifted (no reflection) or off the track.
  if (mask == (LineMask::LEFT | LineMask::CENTER | LineMask::RIGHT)) {
    if (!allBlack) { allBlack = true; allBlackSince = now; }
    if (now - allBlackSince >= LINE_ALL_BLACK_MS) { st = Status::LIFTED; return DRIVE_STOP; }
  } else {
    allBlack = false;
  }

  const int base = LINE_BASE_SPEED_PCT, outer = LINE_OUTER_SPEED_PCT;
  const int soft = LINE_SOFT_INNER_PCT, hard = LINE_HARD_INNER_PCT;

  if (mask != 0) {
    lineSeenAt = now;
    st = Status::FOLLOWING;
    switch (mask) {
      case 0b010: last = makeDrive(base, base);   lastSide = 0;  break;
      case 0b110: last = makeDrive(soft, outer);  lastSide = -1; break;  // line drifting left
      case 0b100: last = makeDrive(hard, outer);  lastSide = -1; break;
      case 0b011: last = makeDrive(outer, soft);  lastSide = +1; break;
      case 0b001: last = makeDrive(outer, hard);  lastSide = +1; break;
      case 0b111: last = makeDrive(base, base);                 break;  // junction
      default:    /* 0b101: ambiguous, keep last */             break;
    }
    return last;
  }

  // mask == 0: no sensor sees the line.
  unsigned long gone = now - lineSeenAt;
  if (gone < LINE_LOST_MS) return last;                     // short gap: coast through
  if (gone < LINE_SEARCH_MAX_MS) {
    st = Status::SEARCHING;
    int8_t dir = lastSide == 0 ? -1 : lastSide;              // default search left
    return dir < 0 ? makeDrive(-LINE_SEARCH_PCT, LINE_SEARCH_PCT)
                   : makeDrive(LINE_SEARCH_PCT, -LINE_SEARCH_PCT);
  }
  st = Status::LOST;
  return DRIVE_STOP;
}
