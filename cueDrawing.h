#ifndef CUE_DRAWING_H
#define CUE_DRAWING_H

#include <UTFT.h>
#include "Arduino.h"
#include <stdint.h>
#include <SPI.h>
#include <Wire.h>



//creates the scren object inside the header file
UTFT myGLCD(SSD1963_800480, 38, 39, 40, 41); //(byte model, int RS, int WR, int CS, int RST, int SER)

//contains [x,y] point of the center of the screen
int center[2] = {};
int xCordBot;
int xCordTop;
int yCordBot;
int yCordTop;

//definitions for touch code that WE DID NOT WRITE
uint8_t addr  = 0x38;  //CTP IIC ADDRESS

// Declare which fonts we will be using
extern uint8_t BigFont[];

#define FT5206_WAKE 11
#define FT5206_INT   48

//UTFT myGLCD(SSD1963_800480,38,39,40,41);  //(byte model, int RS, int WR, int CS, int RST, int SER)
uint16_t tx, ty;

enum {
  eNORMAL = 0,
  eTEST   = 0x04,
  eSYSTEM = 0x01
};

struct TouchLocation {
  uint16_t x;
  uint16_t y;
};

TouchLocation touchLocations[5];

uint8_t readFT5206TouchRegister( uint8_t reg );
uint8_t readFT5206TouchLocation( TouchLocation * pLoc, uint8_t num );
uint8_t readFT5206TouchAddr( uint8_t regAddr, uint8_t * pBuf, uint8_t len );

uint32_t dist(const TouchLocation & loc);
uint32_t dist(const TouchLocation & loc1, const TouchLocation & loc2);

bool sameLoc( const TouchLocation & loc, const TouchLocation & loc2 );


uint8_t buf[30];

uint8_t readFT5206TouchRegister( uint8_t reg ) {
  Wire.beginTransmission(addr);
  Wire.write( reg );  // register 0
  uint8_t retVal = Wire.endTransmission();

  uint8_t returned = Wire.requestFrom(addr, uint8_t(1) );    // request 6 uint8_ts from slave device #2

  if (Wire.available()) {
    retVal = Wire.read();
  }

  return retVal;
}

uint8_t readFT5206TouchAddr( uint8_t regAddr, uint8_t * pBuf, uint8_t len ) {

  Wire.beginTransmission(addr);
  Wire.write( regAddr );  // register 0
  uint8_t retVal = Wire.endTransmission();

  uint8_t returned = Wire.requestFrom(addr, len);    // request 1 bytes from slave device #2

  uint8_t i;

  for (i = 0; (i < len) && Wire.available(); i++) {
    pBuf[i] = Wire.read();
  }

  return i;
}

uint8_t readFT5206TouchLocation( TouchLocation * pLoc, uint8_t num ) {

  uint8_t retVal = 0;
  uint8_t i;
  uint8_t k;

  do {
    if (!pLoc) break; // must have a buffer
    if (!num)  break; // must be able to take at least one

    uint8_t status = readFT5206TouchRegister(2);

    static uint8_t tbuf[40];

    if ((status & 0x0f) == 0) break; // no points detected

    uint8_t hitPoints = status & 0x0f;

//    Serial.print("number of hit points = ");
//    Serial.println( hitPoints );

    readFT5206TouchAddr( 0x03, tbuf, hitPoints * 6);

    for (k = 0, i = 0; (i < hitPoints * 6) && (k < num); k++, i += 6) {
      pLoc[k].x = (tbuf[i + 0] & 0x0f) << 8 | tbuf[i + 1];
      pLoc[k].y = (tbuf[i + 2] & 0x0f) << 8 | tbuf[i + 3];
    }

    retVal = k;

  } while (0);

  return retVal;
}

void writeFT5206TouchRegister( uint8_t reg, uint8_t val) {

  Wire.beginTransmission(addr);
  Wire.write( reg );  // register 0
  Wire.write( val );  // value

  uint8_t retVal = Wire.endTransmission();
}

void readOriginValues(void) {

  writeFT5206TouchRegister(0, eTEST);
  delay(1);

  //uint8_t originLength = readFT5206TouchAddr(0x08, buf, 8 );
  uint8_t originXH = readFT5206TouchRegister(0x08);
  uint8_t originXL = readFT5206TouchRegister(0x09);
  uint8_t originYH = readFT5206TouchRegister(0x0a);
  uint8_t originYL = readFT5206TouchRegister(0x0b);

  uint8_t widthXH  = readFT5206TouchRegister(0x0c);
  uint8_t widthXL  = readFT5206TouchRegister(0x0d);
  uint8_t widthYH  = readFT5206TouchRegister(0x0e);
  uint8_t widthYL  = readFT5206TouchRegister(0x0f);

  //if (originLength)
  {
    Serial.print("Origin X,Y = ");
    Serial.print( uint16_t((originXH << 8) | originXL) );
    Serial.print(", ");
    Serial.println( uint16_t((originYH << 8) | originYL) );

    Serial.print("Width X,Y = ");
    Serial.print( uint16_t((widthXH << 8) | widthXL) );
    Serial.print(", ");
    Serial.println( uint16_t((widthYH << 8) | widthYL) );
  }

}

uint32_t dist(const TouchLocation & loc) {
  uint32_t retVal = 0;

  uint32_t x = loc.x;
  uint32_t y = loc.y;

  retVal = x * x + y * y;

  return retVal;
}

uint32_t dist(const TouchLocation & loc1, const TouchLocation & loc2) {
  uint32_t retVal = 0;

  uint32_t x = loc1.x - loc2.x;
  uint32_t y = loc1.y - loc2.y;

  retVal = sqrt(x * x + y * y);

  return retVal;
}

bool sameLoc( const TouchLocation & loc, const TouchLocation & loc2 ) {
  return dist(loc, loc2) < 50;
}

//Code under here was original

//struct defining a point variable that holds an x and y coordinate
struct Point
{
  int x;
  int y;
};

//struct of points defining the coordinates of the top left, top right, bottom left, and bototm right corners of the LCD
struct cornerPos
{
  struct Point topLeft;
  struct Point topRight;
  struct Point bottomLeft;
  struct Point bottomRight;
};

struct cornerPos corners;

//draws a horizontally striped cue in the following positions [0, 1, 2, 3]
//
void drawHorizontal(int position) {
  int xSize = myGLCD.getDisplayXSize();
  int ySize = myGLCD.getDisplayYSize();

  corners = {20, 20, xSize - 20, 20, 20, ySize - 20, xSize - 20, ySize - 20};
  int recSize;
  int modifier;
  int moveBy;
  int xCord;
  int yCord;

  if (position == 0 | position == 1) {

    if (position == 0) {
      recSize = 150;
      xCord = corners.topLeft.x;
      yCord = corners.topLeft.y;

      modifier = 25;
      moveBy = 30;
    }
    else {
      recSize = -150;
      xCord = corners.topRight.x;
      yCord = corners.topRight.y;
      modifier = 25;
      moveBy = 30;
    }
  }
  else {

    if (position == 2) {
      recSize = 150;
      xCord = corners.bottomLeft.x;
      yCord = corners.bottomLeft.y;
      modifier = -25;
      moveBy = -30;
    }
    else {
      recSize = -150;
      xCord = corners.bottomRight.x;
      yCord = corners.bottomRight.y;
      modifier = -25;
      moveBy = -30;
    }
  }


  //draw horizontal cue
  myGLCD.setColor(VGA_WHITE);
  int xPosStart = xCord;
  int xPosEnd = xCord + recSize;
  int yPosStart = yCord;
  int yPosEnd = yCord + modifier;
  for (int cP = 0; cP < 7; cP++) {
    myGLCD.fillRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
    yPosStart += moveBy;
    yPosEnd += moveBy;
  }
}

//draw vertically striped cue (Fixed!)
void drawVerticalNew(int position) {
  int xSize = myGLCD.getDisplayXSize();
  int ySize = myGLCD.getDisplayYSize();

  corners = {20, 20, xSize - 20, 20, 20, ySize - 20, xSize - 20, ySize - 20};
  int recSize = 0;
  int modifier = 0;
  int moveBy = 0;
  int xCord = 0;
  int yCord = 0;

  if (position == 0 | position == 1) {

    if (position == 0) {
      recSize = 200;
      xCord = corners.topLeft.x;
      yCord = corners.topLeft.y;

      modifier = 25;
      moveBy = 30;
    }
    else {
      recSize = 200;
      xCord = corners.topRight.x;
      yCord = corners.topRight.y;
      modifier = -25;
      moveBy = -30;
    }
  }
  else {
    if (position == 2) {
      recSize = -200;
      xCord = corners.bottomLeft.x;
      yCord = corners.bottomLeft.y;
      modifier = 25;
      moveBy = 30;
    }
    else {
      recSize = -200;
      xCord = corners.bottomRight.x;
      yCord = corners.bottomRight.y;
      modifier = -25;
      moveBy = -30;
    }
  }
  //draw vertical cue
  myGLCD.setColor(VGA_WHITE);
  int xPosStart = xCord;
  int xPosEnd = xCord + modifier;
  int yPosStart = yCord;
  int yPosEnd = yCord + recSize;
  for (int cP = 0; cP < 5; cP++) {
    myGLCD.fillRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
    xPosStart += moveBy;
    xPosEnd += moveBy;
  }
}

//draws a white rectangle with black rectangles inside it in the following positions [0 1]
//
void drawBoxWith(int position) {
  int xSize = myGLCD.getDisplayXSize();
  int ySize = myGLCD.getDisplayYSize();

  corners = {20, 20, xSize - 20, 20, 20, ySize - 20, xSize - 20, ySize - 20};
  int recSizeX;
  int recSizeY;
  int adjustDown;
  int moveBy;
  int xCord;
  int yCord;
  int xEnd;
  int xStart;
  int yStart;
  int yEnd;
  int xPosStart;
  int xPosEnd;
  int yPosStart;
  int yPosEnd;
  int moveByY;
  int moveByX;
  int modifyXStart;
  int modifyXEnd;

  if (position == 0 | position == 1) {

    if (position == 0) {
      recSizeX = 150;
      recSizeY = 200;
      adjustDown = 20;
      xCord = corners.topLeft.x;
      yCord = corners.topLeft.y;
      moveByY = 120;
      moveByX = -80;
      modifyXStart =  130;
      modifyXEnd = -50;

      //moveBy = 30;
    }
    else {
      recSizeX = -150;
      recSizeY = 200;
      adjustDown = 20;
      xCord = corners.topRight.x;
      yCord = corners.topRight.y;
      moveByY = 120;
      moveByX = 80;
      modifyXStart =  -130;
      modifyXEnd = 50;
    }
  }
  else {

    if (position == 2) {
      recSizeX = 150;
      recSizeY = -200;
      adjustDown = -20;
      xCord = corners.bottomLeft.x;
      yCord = corners.bottomLeft.y;
      moveByY = -120;
      moveByX = -80;
      modifyXStart = 130;
      modifyXEnd = -50;
    }
    else {
      recSizeX = -150;
      recSizeY = -200;
      adjustDown = -20;
      xCord = corners.bottomRight.x;
      yCord = corners.bottomRight.y;
      moveByY = -120;
      moveByX = 80;
      modifyXStart =  -130;
      modifyXEnd = 50;
    }
  }

  //draw box with boxes inside
  myGLCD.setColor(VGA_WHITE);
  xPosStart = xCord;
  xPosEnd = xCord + recSizeX;
  yPosStart = yCord;
  yPosEnd = yCord + recSizeY;
  myGLCD.fillRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
  myGLCD.setColor(VGA_BLACK);

  yStart = yPosStart + adjustDown;
  yEnd = yStart + (adjustDown * 2) ;
  for (int row = 0; row < 2; row++) {
    xStart = xPosStart + modifyXStart;
    xEnd = xPosEnd + modifyXEnd;

    if (row > 0) {
      yStart += moveByY;
      yEnd += moveByY;
    }
    for (int col = 0; col < 2; col++) {

      myGLCD.fillRect(xStart, yStart, xEnd, yEnd);

      xStart += moveByX;
      xEnd += moveByX;
    }
  }
}

//draws a white outlined rectangle with white rectangles inside it in the following positions [0 1]
//
void drawBoxWithInverse(int position) {
  int xSize = myGLCD.getDisplayXSize();
  int ySize = myGLCD.getDisplayYSize();

  corners = {20, 20, xSize - 20, 20, 20, ySize - 20, xSize - 20, ySize - 20};
  int recSizeX;
  int recSizeY;
  int adjustDown;
  int moveBy;
  int xCord;
  int yCord;
  int xEnd;
  int xStart;
  int yStart;
  int yEnd;
  int xPosStart;
  int xPosEnd;
  int yPosStart;
  int yPosEnd;
  int moveByY;
  int moveByX;
  int modifyXStart;
  int modifyXEnd;

  if (position == 0 | position == 1) {

    if (position == 0) {
      recSizeX = 150;
      recSizeY = 200;
      adjustDown = 20;
      xCord = corners.topLeft.x;
      yCord = corners.topLeft.y;
      moveByY = 120;
      moveByX = -80;
      modifyXStart =  130;
      modifyXEnd = -50;

      //moveBy = 30;
    }
    else {
      recSizeX = -150;
      recSizeY = 200;
      adjustDown = 20;
      xCord = corners.topRight.x;
      yCord = corners.topRight.y;
      moveByY = 120;
      moveByX = 80;
      modifyXStart =  -130;
      modifyXEnd = 50;
    }
  }
  else {

    if (position == 2) {
      recSizeX = 150;
      recSizeY = -200;
      adjustDown = -20;
      xCord = corners.bottomLeft.x;
      yCord = corners.bottomLeft.y;
      moveByY = -120;
      moveByX = -80;
      modifyXStart = 130;
      modifyXEnd = -50;
    }
    else {
      recSizeX = -150;
      recSizeY = -200;
      adjustDown = -20;
      xCord = corners.bottomRight.x;
      yCord = corners.bottomRight.y;
      moveByY = -120;
      moveByX = 80;
      modifyXStart =  -130;
      modifyXEnd = 50;
    }
  }

  //draw box with boxes inside
  myGLCD.setColor(VGA_WHITE);
  xPosStart = xCord;
  xPosEnd = xCord + recSizeX;
  yPosStart = yCord;
  yPosEnd = yCord + recSizeY;
  myGLCD.drawRect(xPosStart, yPosStart, xPosEnd, yPosEnd);

  yStart = yPosStart + adjustDown;
  yEnd = yStart + (adjustDown * 2) ;
  for (int row = 0; row < 2; row++) {
    xStart = xPosStart + modifyXStart;
    xEnd = xPosEnd + modifyXEnd;

    if (row > 0) {
      yStart += moveByY;
      yEnd += moveByY;
    }
    for (int col = 0; col < 2; col++) {

      myGLCD.fillRect(xStart, yStart, xEnd, yEnd);

      xStart += moveByX;
      xEnd += moveByX;
    }
  }
}

//draws an arrow in the given orientation
void drawArrow(int orientation) {
  //orientation of 1 will be a right arrow
  //orientation of 2 will be a down arrow
  //orientation of 3 will be a left arrow
  //orientation of 4 will be an up arrow
  int xCord;
  int yCord;
  int xCordBot;
  int xCordTop;
  int yCordBot;
  int yCordTop;

  myGLCD.setColor(VGA_WHITE);
  if (orientation == 1) {
    xCord = center[0];
    yCordBot = center[1] - 30;
    yCordTop = center[1] + 30;
    myGLCD.drawLine(xCord, yCordBot, xCord, yCordTop);

    for (int cL = 0; cL < 15; cL++) {
      xCord += 1;
      yCordTop -= 2;
      yCordBot += 2;
      myGLCD.drawLine(xCord, yCordBot, xCord, yCordTop);
    }
    myGLCD.fillRect(center[0] - 50, center[1] - 15, center[0], center[1] + 15);
  }
  if (orientation == 2) {
    //draw vertical arrow
    yCord = center[1] + 100;
    xCordBot = center[0] - 50;
    xCordTop = center[0] + 10;
    myGLCD.drawLine(xCord, yCordBot, xCord, yCordTop);

    for (int cL = 0; cL < 15; cL++) {
      yCord += 1;
      xCordTop -= 2;
      xCordBot += 2;
      myGLCD.drawLine(xCordBot, yCord, xCordTop, yCord);
    }
    myGLCD.fillRect(center[0] - 35, center[1] + 40, center[0] - 5, center[1] + 100);
  }

  if (orientation == 3) {
    xCord = center[0] - 50;
    yCordBot = center[1] - 30;
    yCordTop = center[1] + 30;
    myGLCD.drawLine(xCord, yCordBot, xCord, yCordTop);
    for (int cL = 0; cL < 15; cL++) {
      xCord -= 1;
      yCordTop -= 2;
      yCordBot += 2;
      myGLCD.drawLine(xCord, yCordBot, xCord, yCordTop);
    }
    myGLCD.fillRect(center[0] - 50, center[1] - 15, center[0], center[1] + 15);
  }
  if (orientation == 4) {
    yCord = center[1] + 40;
    xCordBot = center[0] - 50;
    xCordTop = center[0] + 10;
    myGLCD.drawLine(xCord, yCordBot, xCord, yCordTop);

    for (int cL = 0; cL < 15; cL++) {
      yCord -= 1;
      xCordTop -= 2;
      xCordBot += 2;
      myGLCD.drawLine(xCordBot, yCord, xCordTop, yCord);
    }
    myGLCD.fillRect(center[0] - 35, center[1] + 40, center[0] - 5, center[1] + 100);
  }
}

//draws two arrows corresponding to the position of the correct cue
void drawArrowPos(int orientation) {
  if (orientation == 0) {
    drawArrow(3);
    drawArrow(4);
  }
  if (orientation == 1) {
    drawArrow(1);
    drawArrow(4);
  }

  if (orientation == 2) {
    drawArrow(3);
    drawArrow(2);
  }

  if (orientation == 3) {
    drawArrow(1);
    drawArrow(2);
  }
}

void drawCuesCenter() {

  //draw horizontal cue
  int xPosStart;
  int xPosEnd;
  int yPosStart;
  int yPosEnd;

  //draw horizontal cue
  myGLCD.setColor(VGA_WHITE);
  xPosStart = center[0] - 350;
  xPosEnd = center[0] - 200;
  yPosStart = center[1] - 100;
  yPosEnd = center[1] - 75;

  for (int cP = 0; cP < 7; cP++) {
    myGLCD.fillRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
    yPosStart += 30;
    yPosEnd += 30;
  }

  myGLCD.setColor(VGA_WHITE);
  //draw vertical cue
  xPosStart = center[0] - 175;
  xPosEnd = center[0] - 150;
  yPosStart = center[1] - 100;
  yPosEnd = yPosStart + 200;

  for (int cP = 0; cP < 5; cP++) {
    myGLCD.fillRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
    xPosStart += 30;
    xPosEnd += 30;
  }

  //draw box with boxes inside
  myGLCD.setColor(VGA_WHITE);
  xPosStart = center[0];
  xPosEnd = xPosStart + 150;
  yPosStart = center[1] - 100;
  yPosEnd = yPosStart + 200;
  myGLCD.fillRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
  myGLCD.setColor(VGA_BLACK);
  int xEnd;
  int xStart;
  int yStart;
  int yEnd;

  yStart = yPosStart + 20;
  yEnd = yStart + 40 ;
  for (int row = 0; row < 2; row++) {
    xStart = xPosStart + 130;
    xEnd = xPosEnd - 50;

    if (row > 0) {
      yStart += 120;
      yEnd += 120;
    }
    for (int col = 0; col < 2; col++) {

      myGLCD.fillRect(xStart, yStart, xEnd, yEnd);

      xStart -= 80;
      xEnd -= 80;
    }
  }

  //draw box with boxes inside
  myGLCD.setColor(VGA_WHITE);
  xPosStart = center[0] + 175;
  xPosEnd = xPosStart + 150;
  yPosStart = center[1] - 100;
  yPosEnd = yPosStart + 200;
  myGLCD.drawRect(xPosStart, yPosStart, xPosEnd, yPosEnd);
  myGLCD.setColor(VGA_WHITE);
  xEnd;
  xStart;
  yStart;
  yEnd;

  yStart = yPosStart + 20;
  yEnd = yStart + 40 ;
  for (int row = 0; row < 2; row++) {
    xStart = xPosStart + 130;
    xEnd = xPosEnd - 50;

    if (row > 0) {
      yStart += 120;
      yEnd += 120;
    }
    for (int col = 0; col < 2; col++) {

      myGLCD.fillRect(xStart, yStart, xEnd, yEnd);

      xStart -= 80;
      xEnd -= 80;
    }
  }
}

#endif
