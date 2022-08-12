#include "LedControl.h"
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "Wtv020sd16p.h"
#include "LifeCounter.h"

const uint8_t latchPin = 7;
const uint8_t clockPin = 8;
const uint8_t dataPin = 9;
const uint8_t ledPin=6;
LifeCounter lifeCounter(latchPin,clockPin,dataPin,ledPin);

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const uint8_t redButtonPin = 2;
const uint8_t GreenButtonPin = 3;
uint8_t buttonPin = 0;
uint8_t playerBoard[8][8];

LedControl lc = LedControl(12, 11, 10, 4);

int enemyBoard[8][8];
int spotsBombed[8][8];

uint8_t hitCounter = 0;
uint8_t placesToHit = 0;

const uint8_t resetPin = A0;  // The pin number of the reset pin.
const uint8_t clockPinSound = A1;  // The pin number of the clock pin.
const uint8_t dataPinSound = A2;  // The pin number of the data pin.
const uint8_t busyPin = A3;  // The pin number of the busy pin.
Wtv020sd16p wtv020sd16p(resetPin, clockPinSound, dataPinSound, busyPin);

const uint8_t setupMusic = 0;
const uint8_t mainMusic = 1;
const uint8_t VictoryMusic = 4;

void(* resetFunc) (void) = 0;

void setup() {
  //pinMode(5,OUTPUT);
  //analogWrite(5,150);
  wtv020sd16p.reset();

  uint8_t devices = lc.getDeviceCount();
  for (uint8_t address = 0; address < devices; address++) {
    lc.shutdown(address, false);
    lc.setIntensity(address, 8);
    lc.clearDisplay(address);
  }

  pinMode(redButtonPin, INPUT_PULLUP);
  pinMode(GreenButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(redButtonPin), redButtonPress, LOW);
  attachInterrupt(digitalPinToInterrupt(GreenButtonPin), GreenButtonPress, LOW);

  lcd.init();
  lcd.backlight();

  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j ++) {
      playerBoard[i][j] = 0;
      spotsBombed[i][j] = 0;
      enemyBoard[i][j] = 0;
    }
  }
  placesToHit = 9;
  lifeCounter.begin();
  
  printStringScrolling(F("Initialising Naval Base... "));
  Serial.begin(9600);
  delay(1000);
  printStringStatic(F("Set up your"), F("destroyer"));
  wtv020sd16p.simPlayVoice(setupMusic);
  delay(2000);
  setBoat(2);
  lifeCounter.LifeCount(7);
  printStringStatic(F("Set up your"), F("cruiser"));
  delay(2000);
  setBoat(3);
  lifeCounter.LifeCount(4);
  printStringStatic(F("Set up your"), F("battleship"));
  delay(2000);
  setBoat(4);
  lifeCounter.LifeCount(0);
  delay(1000);

  communicateBoards();
  
  printStringStatic(F("BATTLE BEGIN!!!"), F("GOOD LUCK!!!"));
  wtv020sd16p.stopVoice();
  delay(2000);
}

void loop() {
  wtv020sd16p.simPlayVoice(mainMusic);
  bool validInput = false;
  int col;
  int row;

  while (!validInput) {
    wtv020sd16p.simPlayVoice(mainMusic);
    printStringStatic(F("RED = select COL"), F("Green = Confirm"));
    col = getBombCol();
    printStringStatic(F("RED = select ROW"), F("Green = Confirm"));
    row = getBombRow(col);

    if (spotsBombed[row][col] == 1) {
      printStringScrolling(F("You have already bombed that spot, "));
    }
    else {
      spotsBombed[row][col] = 1;
      validInput = true;
    }
  }

  Serial.write(B1);

  printStringStatic(F("WAITING FOR"), F("OPPONENT"));
  while (Serial.available() != 1)
  {
    wtv020sd16p.simPlayVoice(mainMusic);
  }

  Serial.read(); //process B1
  printStringStatic(F("ENEMY HAS ALSO"), F("MADE MOVE"));
  delay(3000);
  lcd.clear();

  if (enemyBoard[row][col] == 1) {
    placesToHit--;
    hitCounter++;
    Serial.write(9 - placesToHit);
    delay(2);
    printStringScrolling("You have now hit " + String(hitCounter) + " spot(s). Only " + String(placesToHit) + " left! ");
  }
  else {
    printStringScrolling("Miss: you still have " + String(placesToHit) + " spot(s) left. ");
  }

  if (placesToHit <= 0) {
    printStringStatic(F("VICTORY!"), F("Red: Reset!"));
    buttonPin = 0;
    wtv020sd16p.stopVoice();
    while (1)
    {
      wtv020sd16p.simPlayVoice(VictoryMusic);
      if (buttonPin == 1)
      {
        resetFunc();
      }
    }
  }
  if (Serial.available() > 0) {
    byte Data = Serial.read();
    lifeCounter.LifeCount(Data);
    if (Data == 9){
      printStringStatic(F("You Lost!"), F("Red: Reset!"));
      buttonPin = 0;
      wtv020sd16p.stopVoice();

      while (1)
      {
        wtv020sd16p.simPlayVoice(VictoryMusic);
        if (buttonPin == 1)
        {
          resetFunc();
        }
      }
    }
  }
}

void printStringScrolling(String myString) {
  uint8_t start = 0;
  uint8_t last = 0;

  for (uint8_t i = 0; i < myString.length() + 16; i ++) {
    lcd.clear();
    lcd.print(myString.substring(start, last));
    if (last < 16) {
      last++;
    }
    else if (last == myString.length() - 1) {
      start++;
    }
    else {
      start++; last++;
    }
    delay(180);
  }
}

void setBoat (uint8_t boatLength) {

  uint8_t boatOrientation;
  buttonPin = 0;
  printStringStatic(F("Green = Vertical"), F("Red = Horizontal"));
  while (1)
  {
    wtv020sd16p.simPlayVoice(setupMusic);
    if (buttonPin == 2) {
      boatOrientation = 2;
      printStringStatic(F("Vertical ship"), F("selected."));
      delay(2000);
      lcd.clear();
      delay(1000);
      break;
    }
    else if (buttonPin == 1) {
      boatOrientation = 1;
      printStringStatic(F("Horizontal ship"), F("selected."));
      delay(2000);
      lcd.clear();
      delay(1000);
      break;
    }
    delay(100);
  }
  printStringStatic(F("Set horizontal"), F("position..."));
  delay(2000);
  printStringStatic(F("RED = Move"), F("Green = Confirm"));
  int boatCol = setBoatHorizontally(boatOrientation, boatLength);
  printStringStatic(F("Set vertical"), F("position..."));
  delay(2000);
  printStringStatic(F("RED = Move"), F("Green = Confirm"));
  setBoatVertically(boatOrientation, boatLength, boatCol);
}


int setBoatHorizontally(int orientation, int boatLength) {
  bool done;
  int currCol;
  int tempBoard[8][8];
  if (orientation == 1) { //horizontal

    int boat[boatLength];
    for (int i = 0; i < boatLength; i++)
      boat[i] = i;

    currCol = 0;
    buttonPin = 0;
    while (1) {
      wtv020sd16p.simPlayVoice(setupMusic);
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          tempBoard[i][j] = playerBoard[i][j];
        }
      }

      for (int i = 0; i < boatLength; i++)
        tempBoard[0][boat[i]] = 1;

      printBoardOnLEDMatrix(tempBoard);


      if (buttonPin == 2) {
        return currCol;
      }
      else if (buttonPin == 1) {
        currCol++;
        buttonPin = 0;
        if (currCol + boatLength > 8) {
          currCol = 0; //reset column (warp edges)
        }
        for (int i = 0; i < boatLength; i++) {
          tempBoard[0][boat[i]] = 0;
          boat[i] = currCol + i;
          tempBoard[0][currCol + i] = 1;
        }
      }
    }
  }
  else if (orientation == 2) { //vertically
    currCol = 0;
    buttonPin = 0;
    while (1) {
      wtv020sd16p.simPlayVoice(setupMusic);
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          tempBoard[i][j] = playerBoard[i][j];
        }
      }

      for (int i = 0; i < boatLength; i++)
        tempBoard[i][currCol] = 1;

      printBoardOnLEDMatrix(tempBoard);

      if (buttonPin == 2) {
        return currCol;
      }
      else if (buttonPin == 1) {
        for (int i = 0; i < boatLength; i++) {
          tempBoard[i][currCol] = 0;
        }
        currCol++;
        buttonPin = 0;
        if (currCol >= 8) {
          currCol = 0;
        }
        for (int i = 0; i < boatLength; i++) {
          tempBoard[i][currCol] = 1;
        }
      }
    }
  }
}

void printBoardOnLEDMatrix(int tempBoard[8][8]) {
  for (int z = 0; z < 5; z++) {
    for (int i = 0; i < 8; i++) {

      byte data = B0;
      for (int j = 0; j < 8; j++) {
        lc.setLed(0, i, j, tempBoard[i][j]);
      }

      delay(1);
    }
  }
}

void setBoatVertically(int orientation, int boatLength, int boatCol) {
  bool done;
  int currRow;
  int tempBoard[8][8];

  if (orientation == 2) { //vertical
    int boat[boatLength];
    for (int i = 0; i < boatLength; i++) {
      boat[i] = i;
    }

    done = false;
    currRow = 0;
    buttonPin = 0;

    while (!done) {
      wtv020sd16p.simPlayVoice(setupMusic);
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          tempBoard[i][j] = playerBoard[i][j];
        }
      }

      for (int i = 0; i < boatLength; i++)
        tempBoard[boat[i]][boatCol] = 1;


      printBoardOnLEDMatrix(tempBoard);

      if (buttonPin == 2) {
        bool validBoat = true;
        for (int i = 0; i < boatLength; i++)
        {
          if (playerBoard[i + currRow][boatCol] == 1)
          {
            printStringScrolling(F("ERROR: Overlap "));
            validBoat = false;
            break;
          }
        }
        if (validBoat)
        {
          for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
              playerBoard[i][j] = tempBoard[i][j];
            }
          }
          done = true;
        }
      }
      else if (buttonPin == 1) {
        currRow++;
        buttonPin = 0;

        if (currRow + boatLength > 8) {
          currRow = 0;
        }
        for (int i = 0; i < boatLength; i++) {
          boat[i] = currRow + i;
        }
      }
    }
  }
  else if (orientation == 1) { // horizontal
    done = false;
    currRow = 0;
    buttonPin = 0;
    while (!done) {
      wtv020sd16p.simPlayVoice(setupMusic);
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          tempBoard[i][j] = playerBoard[i][j];
        }
      }

      for (int i = 0; i < boatLength; i++) {
        tempBoard[currRow][boatCol + i] = 1;
      }

      printBoardOnLEDMatrix(tempBoard);

      if (buttonPin == 2) {
        bool validBoat = true;
        for (int i = 0; i < boatLength; i++)
        {
          if (playerBoard[currRow][boatCol + i] == 1)
          {
            printStringScrolling(F("ERROR: Overlap "));
            validBoat = false;
            break;
          }
        }
        if (validBoat)
        {
          for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
              playerBoard[i][j] = tempBoard[i][j];
            }
          }
          done = true;
        }
      }
      else if (buttonPin == 1) {
        currRow++;
        buttonPin = 0;
        if (currRow >= 8) {
          currRow = 0;
        }
      }
    }
  }
}

int getBombCol() {
  byte bombCol = -1;
  bool done = false;
  int prevVal;
  int tempBoard[8][8];
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      tempBoard[i][j] = spotsBombed[i][j];
    }
  }
  buttonPin = 1;
  while (!done) {
    wtv020sd16p.simPlayVoice(mainMusic);
    prevVal = spotsBombed[0][bombCol];
    tempBoard[0][bombCol] = 1;
    printBoardOnLEDMatrix(tempBoard);
    if (buttonPin == 2) {
      return bombCol;
    }
    else if (buttonPin == 1) {
      tempBoard[0][bombCol] = prevVal;
      bombCol++;
      if (bombCol >= 8) {
        bombCol = 0;
      }
      buttonPin = 0;
    }
  }
}

int getBombRow(int bombCol) {

  int bombRow = -1;
  bool done = false;
  int prevVal;
  int tempBoard[8][8];
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      tempBoard[i][j] = spotsBombed[i][j];
    }
  }
  buttonPin = 1;
  while (!done) {
    wtv020sd16p.simPlayVoice(mainMusic);
    prevVal = spotsBombed[bombRow][bombCol];
    tempBoard[bombRow][bombCol] = 1;
    printBoardOnLEDMatrix(tempBoard);
    if (buttonPin == 2) {
      return bombRow;
    }
    else if (buttonPin == 1) {
      tempBoard[bombRow][bombCol] = prevVal;
      bombRow++;
      if (bombRow >= 8) {
        bombRow = 0;
      }
      buttonPin = 0;
    }
  }
}

void communicateBoards() {

  byte boardToSend[8];
  for (int i = 0; i < 8; i++)
  {
    boardToSend[i] = getRowAsBytes(i);
  }

  Serial.write(boardToSend, 8);
  unsigned long currentMillis=millis();

  printStringStatic(F("WAITING FOR"), F("OPPONENT"));
  while (Serial.available() != 8)
  {
    wtv020sd16p.simPlayVoice(setupMusic);
    if(millis()-currentMillis >=1000){
      Serial.write(boardToSend, 8);
      currentMillis=millis();
    }
  }

  byte boardReceived[8];
  Serial.readBytes(boardReceived, 8);
  for (int i = 0; i < 8; i++)
  {
    byte rowByte = boardReceived[i];
    for (int j = 0; j < 8; j++)
    {
      int x = rowByte & (1 << j);
      if (x != 0)
      {
        x = x >> j;
      }
      enemyBoard[i][j] = x;
    }
  }

  printStringStatic(F("BOARD SETUP"), F("COMPLETE"));
  delay(3000);
  lcd.clear();
}

byte getRowAsBytes(int row)
{
  byte returnVal = B00000000;
  for (int i = 0; i < 8; i++)
  {
    if (playerBoard[row][i] == 1)
    {
      returnVal = returnVal | (B1 << i);
    }
  }
  return returnVal;
}

void printStringStatic(String upper, String lower)
{
  lcd.clear();
  lcd.print(upper);
  lcd.setCursor(0, 1);
  lcd.print(lower);
}

void myDelay(int time)
{
  for (int i = 0; i < time; i++)
  {
    delayMicroseconds(1000);
  }
}

void GreenButtonPress () {
  myDelay(150);
  buttonPin = 2;
}

void redButtonPress () {
  myDelay(150);
  buttonPin = 1;
}