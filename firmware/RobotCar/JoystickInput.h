// JoystickInput.h — KY-023 analog joystick (tethered) on A3/A4.
// Y axis = throttle, X axis = steering, both through the same arcadeMix()
// the Bluetooth keys use. Centre is calibrated at boot; stick inside the
// dead-zone = stop. Readings pinned at 0 or 1023 (unplugged wire, short)
// for JOY_MAX_BAD_READS samples mark the joystick invalid -> motors stop.
#pragma once
#include "Types.h"

class JoystickInput {
public:
  void begin();                                 // calibrates centre if JOYSTICK_FITTED
  void update(unsigned long now);
  bool valid() const;
  DriveCommand drive() const { return valid() ? cmd : DRIVE_STOP; }
  int rawX() const { return x; }
  int rawY() const { return y; }

  // Exposed for the host-side tests in simulation/host-tests.
  static int axisPercent(int raw, int center);
  void setCenterForTest(int cx, int cy) { centerX = cx; centerY = cy; enabled = calibrated = true; }
  void processSample(int rx, int ry);

private:
  bool enabled = false, calibrated = false;
  int x = 512, y = 512, centerX = 512, centerY = 512;
  uint8_t badStreak = 0;
  DriveCommand cmd = DRIVE_STOP;
  unsigned long lastRead = 0;
};
