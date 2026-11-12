#include "Ultrasonic.h"

void Ultrasonic::begin() {
  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);
  digitalWrite(PIN_US_TRIG, LOW);
}

int16_t Ultrasonic::measureOnce() {
  // Echo stuck HIGH before we trigger = wiring fault (or previous echo still running).
  if (digitalRead(PIN_US_ECHO) == HIGH) return INVALID;

  digitalWrite(PIN_US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);

  // Wait for echo to start. A disconnected ECHO wire never rises.
  unsigned long t0 = micros();
  while (digitalRead(PIN_US_ECHO) == LOW) {
    if (micros() - t0 > US_RISE_TIMEOUT_US) return INVALID;
  }
  unsigned long start = micros();
  while (digitalRead(PIN_US_ECHO) == HIGH) {
    if (micros() - start > US_TIMEOUT_US) return US_MAX_CM;   // nothing within range
  }
  unsigned long us = micros() - start;
  uint16_t cm = us / 58;                 // speed of sound, round trip
  if (cm < US_MIN_CM) return INVALID;
  if (cm > US_MAX_CM) cm = US_MAX_CM;
  return cm;
}

static uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
  if (a > b) { uint16_t t = a; a = b; b = t; }
  if (b > c) { uint16_t t = b; b = c; c = t; }
  if (a > b) { uint16_t t = a; a = b; b = t; }
  return b;
}

void Ultrasonic::update(unsigned long now) {
  if (now - lastPing < US_INTERVAL_MS) return;
  lastPing = now;

  int16_t r = measureOnce();
  if (r == INVALID) {
    if (badStreak < 255) badStreak++;
    return;
  }
  badStreak = 0;
  latest = r;
  history[histIdx] = r;
  histIdx = (histIdx + 1) % 3;
  if (goodCount < 3) goodCount++;
  // Until 3 readings exist, fill the median window with the newest value.
  if (goodCount < 3) filtered = r;
  else filtered = median3(history[0], history[1], history[2]);
  samples++;
}
