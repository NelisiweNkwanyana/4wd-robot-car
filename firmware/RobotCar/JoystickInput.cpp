#include "JoystickInput.h"
#include "Config.h"

void JoystickInput::begin() {
  enabled = JOYSTICK_FITTED;
  if (!enabled) return;
  long sx = 0, sy = 0;
  for (uint8_t i = 0; i < 16; i++) { sx += analogRead(PIN_JOY_X); sy += analogRead(PIN_JOY_Y); }
  int cx = sx / 16, cy = sy / 16;
  // Only trust the calibration if the stick was roughly centred at power-on.
  calibrated = abs(cx - 512) <= JOY_CENTER_TOL && abs(cy - 512) <= JOY_CENTER_TOL;
  if (calibrated) { centerX = cx; centerY = cy; }
}

bool JoystickInput::valid() const {
  return enabled && calibrated && badStreak < JOY_MAX_BAD_READS;
}

int JoystickInput::axisPercent(int raw, int center) {
  int d = raw - center;
  if (abs(d) <= (int)JOY_DEADZONE) return 0;
  int span = d > 0 ? (1023 - center) : center;        // distance to that end stop
  span -= JOY_DEADZONE + JOY_EDGE_MARGIN;   // most sticks never reach 0 / 1023
  if (span <= 0) return 0;
  long pct = (long)(abs(d) - JOY_DEADZONE) * 100 / span;
  if (pct > 100) pct = 100;
  return d > 0 ? (int)pct : -(int)pct;
}

void JoystickInput::processSample(int rx, int ry) {
  x = rx; y = ry;
  bool ok = rx >= (int)JOY_VALID_MIN && rx <= (int)JOY_VALID_MAX &&
            ry >= (int)JOY_VALID_MIN && ry <= (int)JOY_VALID_MAX;
  if (!ok) {
    if (badStreak < 255) badStreak++;
    cmd = DRIVE_STOP;
    return;
  }
  badStreak = 0;
  int steer    = axisPercent(rx, centerX);
  int throttle = axisPercent(ry, centerY);
  if (JOY_INVERT_X) steer = -steer;
  if (JOY_INVERT_Y) throttle = -throttle;
  DriveCommand m = arcadeMix(throttle, steer);
  cmd = makeDrive((int)m.left * JOY_MAX_SPEED_PCT / 100, (int)m.right * JOY_MAX_SPEED_PCT / 100);
}

void JoystickInput::update(unsigned long now) {
  if (!enabled || !calibrated) return;
  if (now - lastRead < JOY_INTERVAL_MS) return;
  lastRead = now;
  processSample(analogRead(PIN_JOY_X), analogRead(PIN_JOY_Y));
}
