#include "LifeCounter.h"
#include "Arduino.h"

 LifeCounter::LifeCounter(uint8_t latchPin,uint8_t clockPin,uint8_t dataPin, uint8_t ledPin){
    _latchPin=latchPin;
    _clockPin=clockPin;
    _dataPin=dataPin;
    _ledPin=ledPin;
    pinMode(_latchPin,OUTPUT);
    pinMode(_ledPin,OUTPUT);
 }

  void LifeCounter::begin(){
    LifeCount(9);
  }

void LifeCounter::LifeCount(uint8_t j) {
  digitalWrite(_latchPin, 0);
  shiftOut(dataArray[j]);
  digitalWrite(_latchPin, 1);
  if(j!=9){
    digitalWrite(_ledPin,HIGH);
  }
  else
    digitalWrite(_ledPin,LOW);
}

void LifeCounter::shiftOut(uint8_t myDataOut) {
  bool pinState;
  pinMode(_clockPin, OUTPUT);
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_dataPin, 0);
  digitalWrite(_clockPin, 0);

  for (int i = 7; i >= 0; i--)  {
    digitalWrite(_clockPin, 0);
    if ( myDataOut & (1 << i) ) {
      pinState = 1;
    }
    else {
      pinState = 0;
    }
    digitalWrite(_dataPin, pinState);
    digitalWrite(_clockPin, 1);
    digitalWrite(_dataPin, 0);
  }
  digitalWrite(_clockPin, 0);
}
