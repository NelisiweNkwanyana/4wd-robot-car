#include "Arduino.h"
#include "SoftwareSerial.h"
#include "Servo.h"
#include <cstring>

HardwareSerial Serial;
SoftwareSerial* SoftwareSerial::instance = nullptr;

namespace sim {
  uint64_t nowUs = 0;
  int pinModes[20], digitalOut[20], pwmOut[20], digitalIn[20], analogIn[20];
  Echo echoMode = Echo::NORMAL;
  float echoDistanceCm = 80;
  uint64_t trigFallUs = 0;
  uint8_t trigPin = 11, echoPin = 12;
  int servoAngle = -1; bool servoAttached = false; int servoAttachCount = 0;

  void reset() {
    nowUs = 0;
    memset(pinModes, 0, sizeof pinModes); memset(digitalOut, 0, sizeof digitalOut);
    memset(pwmOut, 0, sizeof pwmOut);     memset(digitalIn, 0, sizeof digitalIn);
    for (int i = 0; i < 20; i++) analogIn[i] = 512;
    echoMode = Echo::NORMAL; echoDistanceCm = 80; trigFallUs = 0;
    servoAngle = -1; servoAttached = false; servoAttachCount = 0;
    Serial.out.clear(); Serial.in.clear();
  }
  void advanceMs(unsigned long ms) { nowUs += (uint64_t)ms * 1000; }
}

static void tick() { sim::nowUs += 4; }   // each pin access costs ~4 us like a real UNO

unsigned long millis() { return (unsigned long)(sim::nowUs / 1000); }
unsigned long micros() { tick(); return (unsigned long)sim::nowUs; }
void delay(unsigned long ms) { sim::advanceMs(ms); }
void delayMicroseconds(unsigned int us) { sim::nowUs += us; }
void pinMode(uint8_t p, uint8_t m) { sim::pinModes[p] = m; }

void digitalWrite(uint8_t p, uint8_t v) {
  tick();
  if (p == sim::trigPin && sim::digitalOut[p] == HIGH && v == LOW) sim::trigFallUs = sim::nowUs;
  sim::digitalOut[p] = v;
}

int digitalRead(uint8_t p) {
  tick();
  if (p == sim::echoPin) {
    using sim::Echo;
    if (sim::echoMode == Echo::NO_ECHO) return LOW;
    if (sim::echoMode == Echo::STUCK_HIGH) return HIGH;
    if (sim::trigFallUs == 0) return LOW;
    uint64_t t = sim::nowUs - sim::trigFallUs;
    const uint64_t rise = 450;  // HC-SR04 sends its 8-cycle burst first
    uint64_t width = sim::echoMode == Echo::OUT_OF_RANGE ? 38000 : (uint64_t)(sim::echoDistanceCm * 58);
    return (t >= rise && t < rise + width) ? HIGH : LOW;
  }
  return sim::digitalIn[p];
}

int analogRead(uint8_t p) { tick(); return sim::analogIn[p]; }
void analogWrite(uint8_t p, int v) { sim::pwmOut[p] = v; }
