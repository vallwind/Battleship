#pragma once
#include <avr/pgmspace.h>

class LifeCounter{
public:
  LifeCounter(uint8_t latchPin,uint8_t clockPin,uint8_t dataPin,uint8_t ledPin);
  void begin();
  void LifeCount(uint8_t j);

  
private:
  uint8_t _latchPin;
  uint8_t _clockPin;
  uint8_t _dataPin;
  uint8_t _ledPin;
  uint8_t dataArray[10]={0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00};
  void shiftOut(uint8_t myDataOut);
};