// 05_bluetooth_test — shows exactly what your phone app sends.
// Pair "HC-05" (PIN 1234 or 0000), connect from the app, press each button.
// Every byte is printed with its timing, so you can see whether the app
// REPEATS a key while a button is held (needed for the 600 ms failsafe).
#include <SoftwareSerial.h>
SoftwareSerial bt(2, 3);   // RX <- HC-05 TXD, TX -> HC-05 RXD (through 1k/2k divider!)
unsigned long last = 0;
void setup() { Serial.begin(115200); bt.begin(9600);
  Serial.println(F("BT test: press app buttons. Type in Serial Monitor to send to phone.")); }
void loop() {
  while (bt.available()) {
    char c = bt.read(); unsigned long now = millis();
    Serial.print(F("rx '")); Serial.print(c >= 32 ? c : '.'); Serial.print(F("' 0x"));
    Serial.print((uint8_t)c, HEX); Serial.print(F("  +")); Serial.print(now - last); Serial.println(F(" ms"));
    last = now;
    bt.print(F("echo ")); bt.println(c);
  }
  while (Serial.available()) bt.write(Serial.read());
}
