#include "Battery.h"
#include "Config.h"

float Battery::presentV() { return BATTERY_PRESENT_V; }

void Battery::begin() {
  if (BATTERY_SENSE_FITTED) pinMode(PIN_BATTERY, INPUT);
}

void Battery::feedVolts(float v, unsigned long now) {
  if (!primed) { filtered = v; primed = true; }
  else filtered += 0.2f * (v - filtered);

  if (!present()) { isLow = false; belowCut = false; return; }   // USB only

  if (filtered < BATTERY_CUTOFF_V) {
    if (!belowCut) { belowCut = true; belowSince = now; }
    if (now - belowSince >= BATTERY_LOW_HOLD_MS) isLow = true;
  } else {
    belowCut = false;
    if (isLow && filtered >= BATTERY_RESUME_V) isLow = false;
  }
}

void Battery::update(unsigned long now) {
  if (!BATTERY_SENSE_FITTED) return;
  if (now - lastRead < BATTERY_INTERVAL_MS) return;
  lastRead = now;
  // ADC is ratiometric to the 5 V rail; BATTERY_CAL corrects small errors.
  float pinV = analogRead(PIN_BATTERY) * (5.0f / 1023.0f);
  feedVolts(pinV * BATTERY_DIVIDER * BATTERY_CAL, now);
}
