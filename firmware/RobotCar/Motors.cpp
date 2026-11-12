#include "Motors.h"
#include "Config.h"

void Motors::begin() {
  const uint8_t pins[] = {PIN_MOTOR_ENA, PIN_MOTOR_ENB, PIN_MOTOR_IN1,
                          PIN_MOTOR_IN2, PIN_MOTOR_IN3, PIN_MOTOR_IN4};
  for (uint8_t p : pins) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }
  stopNow();
}

int16_t Motors::percentToPwm(int8_t pct) {
  if (pct == 0) return 0;
  int16_t mag = abs((int)pct);
  if (mag > 100) mag = 100;
  // 1..100 % -> PWM_MIN_MOVING..PWM_MAX so small requests still turn the wheels
  int16_t pwm = PWM_MIN_MOVING + (int32_t)(PWM_MAX - PWM_MIN_MOVING) * (mag - 1) / 99;
  return pct > 0 ? pwm : -pwm;
}

void Motors::setTarget(const DriveCommand& cmd) {
  int8_t l = INVERT_LEFT_MOTORS  ? -cmd.left  : cmd.left;
  int8_t r = INVERT_RIGHT_MOTORS ? -cmd.right : cmd.right;
  tgtLeft  = percentToPwm(l);
  tgtRight = percentToPwm(r);
}

void Motors::stopNow() {
  tgtLeft = tgtRight = curLeft = curRight = 0;
  apply();
}

int16_t Motors::stepToward(int16_t cur, int16_t target) {
  if (cur == target) return cur;
  // Crossing direction: drop to zero first, never jump straight to reverse.
  if ((cur > 0 && target < 0) || (cur < 0 && target > 0)) target = 0;
  // Starting from rest: jump to the minimum moving PWM (below it the motor only hums).
  // (percentToPwm never produces a non-zero value below PWM_MIN_MOVING.)
  if (cur == 0) return target > 0 ? (int16_t)PWM_MIN_MOVING : -(int16_t)PWM_MIN_MOVING;
  int16_t next = cur + (target > cur ? RAMP_STEP : -RAMP_STEP);
  if ((target > cur && next > target) || (target < cur && next < target)) next = target;
  // Never sit in the stall band between 0 and PWM_MIN_MOVING.
  if (next != 0 && abs(next) < PWM_MIN_MOVING) next = 0;
  return next;
}

void Motors::update(unsigned long now) {
  if (now - lastRamp < RAMP_INTERVAL_MS) return;
  lastRamp = now;
  curLeft  = stepToward(curLeft,  tgtLeft);
  curRight = stepToward(curRight, tgtRight);
  apply();
}

void Motors::writeSide(uint8_t en, uint8_t inA, uint8_t inB, int16_t pwm) {
  if (pwm == 0) {                 // coast: both inputs low, enable off
    analogWrite(en, 0);
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    return;
  }
  digitalWrite(inA, pwm > 0 ? HIGH : LOW);
  digitalWrite(inB, pwm > 0 ? LOW : HIGH);
  analogWrite(en, (uint8_t)abs(pwm));
}

void Motors::apply() {
  writeSide(PIN_MOTOR_ENA, PIN_MOTOR_IN1, PIN_MOTOR_IN2, curLeft);
  writeSide(PIN_MOTOR_ENB, PIN_MOTOR_IN3, PIN_MOTOR_IN4, curRight);
}
