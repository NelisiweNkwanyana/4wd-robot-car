// 07_battery_test — compares the A5 reading with your multimeter.
// BATTERY_CAL in Config.h = multimeter volts / printed volts.
const uint8_t PIN = A5; const float DIVIDER = 2.0;
void setup() { Serial.begin(115200); }
void loop() {
  long sum = 0; for (int i = 0; i < 16; i++) sum += analogRead(PIN);
  float v = sum / 16.0 * 5.0 / 1023.0 * DIVIDER;
  Serial.print(F("battery ~")); Serial.print(v, 2); Serial.println(F(" V  (2S: 8.4 full, 7.4 nominal, 6.4 firmware cut-off)"));
  delay(500);
}
