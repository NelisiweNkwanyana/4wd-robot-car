// 02_line_sensor_test — prints L C R as 0/1 ten times a second.
// Hold the car over white, then over black tape (sensors ~5-10 mm above surface).
// Turn each module's blue potentiometer until its LED and the output flip cleanly.
// If black shows 0 and white shows 1, set LINE_ACTIVE_LOW = true in Config.h.
const uint8_t L = A0, C = A1, R = A2;
void setup() { Serial.begin(115200); pinMode(L, INPUT); pinMode(C, INPUT); pinMode(R, INPUT);
  Serial.println(F("L C R  (raw DO levels)")); }
void loop() {
  Serial.print(digitalRead(L)); Serial.print(' ');
  Serial.print(digitalRead(C)); Serial.print(' ');
  Serial.println(digitalRead(R));
  delay(100);
}
