#include "Scanner.h"
#include "Config.h"
#include <Servo.h>

static Servo servo;

uint8_t Scanner::centerDeg() { return SERVO_CENTER_DEG; }

void Scanner::begin() {
  if (!SERVO_FITTED) return;
  current = SERVO_CENTER_DEG + 1;    // force a real move on first park
  park(0);
}

void Scanner::lookTo(uint8_t deg, unsigned long now) {
  parking = false;
  if (!SERVO_FITTED) { current = deg; movedAt = now; return; }
  if (!attached) { servo.attach(PIN_SERVO); attached = true; }
  if (deg != current) movedAt = now;
  current = deg;
  servo.write(deg);
}

bool Scanner::settled(unsigned long now) const {
  return now - movedAt >= SERVO_SETTLE_MS;
}

void Scanner::update(unsigned long now) {
  if (parking && attached && settled(now)) {
    servo.detach();
    attached = false;
    parking = false;
  }
}
