// 03_ultrasonic_test — distance every 100 ms using the same timeouts as the firmware.
// Check against a tape measure at 10, 20, 50, 100 cm. Unplug ECHO: must print NO-ECHO.
const uint8_t TRIG = 11, ECHO = 12;
void setup() { Serial.begin(115200); pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT); }
void loop() {
  if (digitalRead(ECHO) == HIGH) { Serial.println(F("ECHO STUCK HIGH")); delay(100); return; }
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10); digitalWrite(TRIG, LOW);
  unsigned long t0 = micros();
  while (digitalRead(ECHO) == LOW) if (micros() - t0 > 3000) { Serial.println(F("NO-ECHO (check wiring/5V)")); delay(100); return; }
  unsigned long s = micros();
  while (digitalRead(ECHO) == HIGH) if (micros() - s > 6000) { Serial.println(F(">100 cm (clear)")); delay(100); return; }
  Serial.print((micros() - s) / 58); Serial.println(F(" cm"));
  delay(100);
}
