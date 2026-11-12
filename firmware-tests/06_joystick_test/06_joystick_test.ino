// 06_joystick_test — raw X/Y. Note the centre values, the extremes and which
// way is "forward". Forward should INCREASE Y; if not, set JOY_INVERT_Y = true.
// Right should INCREASE X; if not, set JOY_INVERT_X = true.
const uint8_t X = A3, Y = A4;
void setup() { Serial.begin(115200); }
void loop() {
  Serial.print(F("X=")); Serial.print(analogRead(X));
  Serial.print(F("  Y=")); Serial.println(analogRead(Y));
  delay(100);
}
