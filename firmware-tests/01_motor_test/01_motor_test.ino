// 01_motor_test — one side at a time, low speed, WHEELS OFF THE GROUND.
// Pins match firmware/RobotCar/Config.h. Serial Monitor 115200.
// Keys: 1 left fwd  2 left back  3 right fwd  4 right back  5 both fwd
//       6 both back  + / - speed   any other key = STOP
const uint8_t ENA = 5, IN1 = 4, IN2 = 7, ENB = 6, IN3 = 8, IN4 = 10;
int pwm = 120;

void side(uint8_t en, uint8_t a, uint8_t b, int dir) {
  digitalWrite(a, dir > 0); digitalWrite(b, dir < 0);
  analogWrite(en, dir == 0 ? 0 : pwm);
}
void stopAll() { side(ENA, IN1, IN2, 0); side(ENB, IN3, IN4, 0); }

void setup() {
  const uint8_t pins[] = {ENA, IN1, IN2, ENB, IN3, IN4};   // (no std::initializer_list on AVR)
  for (uint8_t p : pins) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }
  Serial.begin(115200);
  Serial.println(F("Motor test. Wheels OFF the ground. 1-6 move, +/- speed, other = stop. Auto-stop after 2 s."));
}

unsigned long started = 0;
void loop() {
  if (started && millis() - started > 2000) { stopAll(); started = 0; Serial.println(F("auto-stop")); }
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r') return;
  stopAll(); delay(150);                       // never reverse straight from moving
  switch (c) {
    case '1': side(ENA, IN1, IN2, +1); Serial.println(F("LEFT forward"));  break;
    case '2': side(ENA, IN1, IN2, -1); Serial.println(F("LEFT backward")); break;
    case '3': side(ENB, IN3, IN4, +1); Serial.println(F("RIGHT forward")); break;
    case '4': side(ENB, IN3, IN4, -1); Serial.println(F("RIGHT backward"));break;
    case '5': side(ENA, IN1, IN2, +1); side(ENB, IN3, IN4, +1); Serial.println(F("BOTH forward")); break;
    case '6': side(ENA, IN1, IN2, -1); side(ENB, IN3, IN4, -1); Serial.println(F("BOTH backward"));break;
    case '+': pwm = min(255, pwm + 20); Serial.print(F("pwm=")); Serial.println(pwm); return;
    case '-': pwm = max(0, pwm - 20);   Serial.print(F("pwm=")); Serial.println(pwm); return;
    default:  Serial.println(F("STOP")); return;
  }
  started = millis();
}
// Record: lowest PWM where each side starts turning reliably -> PWM_MIN_MOVING.
// If a side spins the wrong way: set INVERT_LEFT/RIGHT_MOTORS in Config.h
// (or swap that side's two motor wires at the L298N).
