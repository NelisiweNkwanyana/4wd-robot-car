// 04_servo_test — sweeps centre / left / right. Watch for Arduino resets
// (the "ready" line printing again) = servo current too high for the 5 V rail.
// Adjust SERVO_*_DEG in Config.h so the sensor points straight ahead at centre.
#include <Servo.h>
Servo s;
const uint8_t PIN = 9;
const int angles[] = {90, 160, 90, 20};
void setup() { Serial.begin(115200); Serial.println(F("servo test ready")); s.attach(PIN); }
void loop() {
  for (int a : angles) { s.write(a); Serial.print(F("angle ")); Serial.println(a); delay(1000); }
  if (Serial.available()) { int a = Serial.parseInt(); if (a >= 0 && a <= 180) { s.write(a); delay(3000); } }
}
