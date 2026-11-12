// LineFollower.h — 3-sensor bang-bang follower with gap tolerance and search.
//
//  L C R   action
//  0 1 0   straight
//  1 1 0   soft left        0 1 1   soft right
//  1 0 0   hard left        0 0 1   hard right   (inner wheel reverses)
//  1 1 1   junction / wide line: straight; persisting = LIFTED fault
//  1 0 1   ambiguous: keep previous command
//  0 0 0   gap: keep previous command for LINE_LOST_MS, then pivot toward the
//          side the line was last seen; after LINE_SEARCH_MAX_MS -> LOST
#pragma once
#include "Types.h"

class LineFollower {
public:
  enum class Status : uint8_t { FOLLOWING, SEARCHING, LOST, LIFTED };

  void reset(unsigned long now);
  DriveCommand update(uint8_t mask, unsigned long now);
  Status status() const { return st; }

private:
  DriveCommand last = DRIVE_STOP;
  int8_t lastSide = 0;            // -1 line was left, +1 right, 0 centre
  unsigned long lineSeenAt = 0;
  unsigned long allBlackSince = 0;
  bool allBlack = false;
  Status st = Status::FOLLOWING;
};
