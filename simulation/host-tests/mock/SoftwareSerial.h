#pragma once
#include "Arduino.h"

class SoftwareSerial : public Stream {
public:
  SoftwareSerial(uint8_t, uint8_t) { instance = this; }
  void begin(unsigned long) {}
  static SoftwareSerial* instance;   // lets tests reach the firmware's static BT port
};
