// Minimal Arduino core mock for running the firmware logic on a PC.
// Time is simulated: every pin read / micros() call advances the clock a
// little, so the firmware's busy-wait loops (ultrasonic echo) make progress.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <deque>
#include <cstdio>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define HEX 16
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper*>(s))

template <class T, class L, class H> T constrain(T x, L lo, H hi) { return x < lo ? lo : (x > hi ? hi : x); }
template <class A, class B> A min(A a, B b) { return a < (A)b ? a : (A)b; }
template <class A, class B> A max(A a, B b) { return a > (A)b ? a : (A)b; }

// ---------------- simulated hardware ----------------
namespace sim {
  extern uint64_t nowUs;
  extern int pinModes[20];
  extern int digitalOut[20];
  extern int pwmOut[20];
  extern int digitalIn[20];
  extern int analogIn[20];
  // Ultrasonic model
  enum class Echo { NORMAL, NO_ECHO, STUCK_HIGH, OUT_OF_RANGE };
  extern Echo echoMode;
  extern float echoDistanceCm;
  extern uint64_t trigFallUs;
  extern uint8_t trigPin, echoPin;
  void reset();
  void advanceMs(unsigned long ms);
}

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);

// ---------------- Print / Serial ----------------
class Print {
public:
  std::string out;
  virtual ~Print() {}
  void write(const std::string& s) { out += s; }
  void print(const char* s) { write(s); }
  void print(const __FlashStringHelper* s) { write(reinterpret_cast<const char*>(s)); }
  void print(char c) { out += c; }
  void print(unsigned char v) { write(std::to_string((unsigned)v)); }
  void print(unsigned char v, int) { write(std::to_string((unsigned)v)); }
  void write(int c) { out += (char)c; }
  void print(int v) { write(std::to_string(v)); }
  void print(unsigned int v) { write(std::to_string(v)); }
  void print(long v) { write(std::to_string(v)); }
  void print(unsigned long v) { write(std::to_string(v)); }
  void print(double v, int digits = 2) { char b[32]; snprintf(b, sizeof b, "%.*f", digits, v); write(b); }
  template <class T> void println(T v) { print(v); out += "\n"; }
  void println(double v, int d) { print(v, d); out += "\n"; }
  void println() { out += "\n"; }
};

class Stream : public Print {
public:
  std::deque<char> in;
  void feed(const std::string& s) { for (char c : s) in.push_back(c); }
  int available() { return (int)in.size(); }
  long parseInt() { return 0; }
  int read() { if (in.empty()) return -1; char c = in.front(); in.pop_front(); return (unsigned char)c; }
};

class HardwareSerial : public Stream {
public:
  void begin(unsigned long) {}
};
extern HardwareSerial Serial;
