/*
   BlueVGA library - VGA Driver Library for STM32F103

   - This library is intended to work in Arduino IDE using Bluepill STM32F103C8 or STM32F103CB boards
   - It works on both STM32 Core and Roger's core for STM32F103C board.
   - It was tested and runs using the following Arduino Settings for the board:
       Generic STM32F103C series
       Optimize Os (Smallest)
       Variant STM32F103C8 or STM32F103CB
       CPU Speed(MHz) 72MHz (Normal)

    Author Rodrigo Patricio Garcia Corbera (rocorbera@gmail.com)
    Copyright © 2017-2021 Rodrigo Patricio Garcia Corbera. 
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    This code is licensed as Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0) - https://creativecommons.org/licenses/by-nc-sa/4.0/
    Redistributions of source code must retain the above copyright notice, and meet al conditions as defined in  https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the this disclaimer in 
    the documentation and/or other materials provided with the distribution.

    ** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. 
    ** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
    ** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

#include <arduino.h>
#include "bluevga.h"
#include "UserDefinedFont.h"

#include "Joystick.h"

// using an Arduino Analog Joystick - VRX Analog Signal on PB0, VRY Analog Signal on PB1 and SW on PB10
// this video at https://www.youtube.com/watch?v=f8iaW5YcsNE shows the joystick used in this scketch
Joystick joystick(PB0, PB0, PB1, PB1, PB10);

// creates a BlueVGA object using GRAPH_ASCII_FONT as bitmap for all the tiles
BlueVGA vga(GRAPH_ASCII_FONT);

// Colors in Red-Green-Blue RGB 3 bits format - 8 possible colors
enum {
  MUSHROOM_COLOR = 0x08,             // BG Black with FG Red
  FLY_COLOR      = 0x0C,             // BG Black with FG Yellow
  BEETLE_COLOR   = 0x06,             // BG Black with FG Cyan
  SNK_HEAD_COLOR = 0x0E,             // BG Black with FG White
  SNK_BODY_COLOR = 0x2A,             // BG Blue with FG Magenta
  BORDER_COLOR   = 0x04,             // BG Black with FG Green
};

enum {
  MUSHROOM_TILE  = 0,
  FLY_TILE       = 1,
  BEETLE_TILE    = 2,
  SNK_HD_UP_TILE = 3,   // Head facing UP
  SNK_HD_DW_TILE = 4,   // Head facing DOWN
  SNK_HD_LT_TILE = 5,   // Head facing LEFT
  SNK_HD_RT_TILE = 6,   // Head facing RIGHT
  SNK_BODY_TILE  = 7,
};


void setup() {
  static int32_t score = 0;
  static int32_t hiscore = 0;
  // game loop ... no need for using arduino loop()
  while (true) {
    vga.clearScreen();     // ASCII 0x20 (blank space) is, by default, used to fill the whole screen. Here we set background color to black, and also black for foreground!
    updateScore(score, hiscore);
    runStarting();
    runGame(score, hiscore);
  }
}


void drawGameScr(void) {
  // little animation for drawing the fence with '#'
  for (uint8_t i = 0; i < 24; i++) {
    vga.setTile(2 + i, 4, '#', BORDER_COLOR);
    vga.setTile(2, 4 + i, '#', BORDER_COLOR);
    vga.setTile(2 + 23 - i, 4 + 23, '#', BORDER_COLOR);
    vga.setTile(2 + 23, 4 + 23 - i, '#', BORDER_COLOR);
    vga.waitVSync(3);
  }
  // draws the snake in its initial position and size
  vga.setTile(2 + 12, 4 + 7, SNK_HD_LT_TILE, SNK_HEAD_COLOR);
  for (uint8_t i = 0; i < 5; i++)  vga.setTile(2 + 13 + i, 4 + 7, SNK_BODY_TILE, SNK_BODY_COLOR);
}

void updateScore(uint16_t sc, uint16_t hi) {
  // top of screen has score and hiscores (yellow and white)
  vga.printStr(0, 2, 0x0C, (char *)"SCORE:000000  HISCORE:000000");
  vga.printInt(6, 2, sc, 0x0E, true, 6);
  vga.printInt(22, 2, hi, 0x0E, true, 6);
}

// little animation for populating random mushroosm, flies and beetles
void populateScr(uint8_t numElements) {
  randomSeed(analogRead(PA4) + vga.getFrameNumber());
  // animation for mushrooms from function paramenter
  while (numElements) {
    uint8_t x = random(3, 3 + 22);
    uint8_t y = random(5, 5 + 22);
    if (vga.getTile(x, y) != ' ') continue;
    vga.setTile(x, y, MUSHROOM_TILE, MUSHROOM_COLOR);
    vga.waitVSync(4);
    numElements--;
  }
  // animation for flies
  numElements = 5;
  while (numElements) {
    uint8_t x = random(3, 3 + 22);
    uint8_t y = random(5, 5 + 22);
    if (vga.getTile(x, y) != ' ') continue;
    vga.setTile(x, y, FLY_TILE, FLY_COLOR);
    vga.waitVSync(4);
    numElements--;
  }
  // animation for beetles
  numElements = 15;
  while (numElements) {
    uint8_t x = random(3, 3 + 22);
    uint8_t y = random(5, 5 + 22);
    if (vga.getTile(x, y) != ' ') continue;
    vga.setTile(x, y, BEETLE_TILE, BEETLE_COLOR);
    vga.waitVSync(4);
    numElements--;
  }
}

// run the game itself...
void runGame(int32_t &score, int32_t &hiscore) {

  score = 0;
  uint8_t headPosX = 2 + 12;
  uint8_t headPosY = 4 + 7;
  int8_t dx = -1, dy = 0;

  uint8_t tailPosX = 2 + 12 + 5;
  uint8_t tailPosY = 4 + 7;
  uint8_t scoreInc = 1;
  uint8_t snakeSpeed = 25;
  uint32_t nextTime = 0;
  uint8_t numFlies = 5;
  uint8_t numBeetles = 15;
  bool dead = false;
  uint8_t snkInc = 0;

  // prepares initial screen drawing
  vga.clearScreen();
  updateScore(score, hiscore);
  drawGameScr();

  populateScr(20);
  vga.waitVSync(120);

  nextTime = snakeSpeed;  // remaining ticks to move the snake in 1/60 of second
  // runs the game while player doesn't get killed
  while (!dead) {
    vga.waitVSync(); // holds up to the start of next frame - it keeps the screen solid and clean

    if (joystick.firePressed()) break; // player may want to quit by pressing joystick switch...

    if (!numBeetles && !numFlies) {  // wave finished? then restart it and speed up the game!
      populateScr(10);
      numFlies = 5;
      numBeetles = 15;
      snakeSpeed -= 5;
      if (snakeSpeed < 10) snakeSpeed = 10;
    }

    // check the joystick... but no move is done until right time...
    if (joystick.rightPressed()) {
      dx = 1;
      dy = 0;
      vga.setTile(headPosX, headPosY, SNK_HD_RT_TILE, SNK_HEAD_COLOR);
    }
    else if (joystick.leftPressed()) {
      dx = -1;
      dy = 0;
      vga.setTile(headPosX, headPosY, SNK_HD_LT_TILE, SNK_HEAD_COLOR);
    }
    else if (joystick.downPressed()) {
      dx = 0;
      dy = 1;
      vga.setTile(headPosX, headPosY, SNK_HD_DW_TILE, SNK_HEAD_COLOR);
    }
    else if (joystick.upPressed()) {
      dx = 0;
      dy = -1;
      vga.setTile(headPosX, headPosY, SNK_HD_UP_TILE, SNK_HEAD_COLOR);
    }

    nextTime--;
    if (nextTime == 0) {  // is it the right time to move the snake?
      nextTime = snakeSpeed;

      // moves snake
      uint8_t nextPositionTile = vga.getTile(headPosX + dx, headPosY + dy);
      bool shouldMove = true;

      // checks what snake is about to eat and if it is going to get positioned
      uint8_t bonus = 0;
      switch (nextPositionTile) {
        case MUSHROOM_TILE:             // mushroom.... sorry, you're dead!
          shouldMove = false;
          dead = true;
          break;
        case FLY_TILE:                 // Fly! increases score faster!
          scoreInc += 1;
          snkInc += 1;                 // snake gets longer
          bonus = (150 + 6 * scoreInc) / 10;
          numFlies--;
        case BEETLE_TILE:             // case follow on
          scoreInc += 1;
          snkInc += 1;
          if (!bonus) {               // was it a Fly or Bettle?
            bonus = (100 + 5 * scoreInc) / 10;
            numBeetles--;
          }
          // Litle animation for eating flies and beetles
          for (uint8_t i = 'A'; i < 'K'; i++) {
            score += bonus;
            if (score > hiscore) hiscore = score;
            updateScore(score, hiscore);
            vga.setTile(headPosX + dx, headPosY + dy, i);  // setTile(...) changes the tile, keeping its original color
            vga.waitVSync(4);
          }
          vga.setTile(headPosX + dx, headPosY + dy, ' ');
          break;
        default:  // did you get to the fence?  are you going to eat yourself?
          if (nextPositionTile == '#' || nextPositionTile == SNK_BODY_TILE ||
              vga.getTile(headPosX + dx + 1, headPosY + dy) == SNK_BODY_TILE ||
              vga.getTile(headPosX + dx - 1, headPosY + dy) == SNK_BODY_TILE ||
              vga.getTile(headPosX + dx, headPosY + dy - 1) == SNK_BODY_TILE ||
              vga.getTile(headPosX + dx, headPosY + dy + 1) == SNK_BODY_TILE) {
            shouldMove = false;
            score -= 10;                      // you don't get killed, but your score decreases
            if (score < 0) dead = true;
            else updateScore(score, hiscore);
          }
      }

      if (!numBeetles && !numFlies) {        // very special case...
        shouldMove = false;  // a chance to eat that hidden insect!
      }

      if (shouldMove) {           // moves the snake!
        // moves snake head
        uint8_t lastSnkHead = vga.getTile(headPosX, headPosY);
        vga.setTile(headPosX, headPosY, SNK_BODY_TILE, SNK_BODY_COLOR);
        headPosX += dx;
        headPosY += dy;
        vga.setTile(headPosX, headPosY, lastSnkHead, SNK_HEAD_COLOR);
        // moves tail if body size has not grown
        if (snkInc) snkInc--;
        else {
          // moves tail and finds tail again
          vga.setTile(tailPosX, tailPosY, ' ', 0);
          if (vga.getTile(tailPosX - 1, tailPosY) == SNK_BODY_TILE)  tailPosX -= 1;
          else if (vga.getTile(tailPosX + 1, tailPosY) == SNK_BODY_TILE)  tailPosX += 1;
          else if (vga.getTile(tailPosX, tailPosY - 1) == SNK_BODY_TILE)  tailPosY -= 1;
          else if (vga.getTile(tailPosX, tailPosY + 1) == SNK_BODY_TILE)  tailPosY += 1;
        }
      }
    }
  }
  // died... some colorful nice video effect with background color...
  for (int8_t i = 25; i >= 0; i--) {
    for (uint8_t y = 5; y < 27; y++)
      for (uint8_t x = 3; x < 25; x++)
        if (vga.getTile(x, y) != SNK_BODY_TILE) vga.setBGColor(x, y, ((i*2)&0xF)); // (i*2)&0xF colors are even numbers from 0 to 14!
    vga.waitVSync(4);
  }
}


void runStarting(void) {
  vga.printStr(5, 5, 0x08, (char *)"S N A K E - B Y T E");
  vga.printStr(5, 6, 0x0A, (char *)"===================");

  uint8_t timeCounter = 0;

  while (!joystick.anyPressed())  {
    vga.waitVSync(6);
    timeCounter++;  // 10 is 1 second

    if (timeCounter < 60) { // 6 seconds = 60 * 6 = 360 / 60 FPS
      vga.setColorRegion(0, 8, VRAM_WIDTH - 1, VRAM_HEIGHT - 4, 0); // just a trick to erase a part of the screen...
      vga.printStr(0, 8, 0x0C, (char *)"Based on the  VIC 20 version");
      vga.printStr(3, 9, 0x0C, (char *)"Your Computer Magazine");
      vga.printStr(1, 10, 0x0C, (char *)"April, 1983  by Dilley, A.");

      vga.printStr(1, 14, 0x06, (char *)"adapted by Rodrigo Corbera");

      vga.printStr(3, 16, 0x0E, (char *)"BluePill STM32F103C8T6");
      vga.printStr(3, 17, 0x0E, (char *)"using  BlueVGA Library");
      vga.printStr(1, 18, 0x0E, (char *)"www.rogabe.art.br/BlueVGA/");

      vga.printStr(2, 22, 0x04, (char *)"Licensed CC BY-NC-SA 4.0");
      vga.printStr(0, 23, 0x04, (char *)"github.com/rocorbera/bluevga");
    } else if (timeCounter < 120) { //  // 120 - 60 = 60 ==> 6 seconds = 60 * 6 = 360 / 60 FPS
      vga.setColorRegion(0, 8, VRAM_WIDTH - 1, VRAM_HEIGHT - 4, 0); // just a trick to erase a part of the screen...
      vga.printStr(2, 8, 0x0E, (char *)"Deadly Poisoned Mushroom");
      vga.setTile(0, 8, MUSHROOM_TILE, MUSHROOM_COLOR);
      vga.printStr(2, 10, 0x0E, (char *)"Eat First for Higher Score");
      vga.setTile(0, 10, FLY_TILE, FLY_COLOR);
      vga.printStr(2, 12, 0x0E, (char *)"Has a Good Taste!");
      vga.setTile(0, 12, BEETLE_TILE, BEETLE_COLOR);

      vga.printStr(0, 14, 0x0A, (char *)"Drive Snake Avoid Mushrooms!");
      vga.setTile(6, 14, SNK_HD_LT_TILE, SNK_HEAD_COLOR);
      for (uint8_t x = 7; x < 11; x++)  vga.setTile(x, 14, SNK_BODY_TILE, SNK_BODY_COLOR);

      vga.printStr(2, 17, 0x06, (char *)"Eat All Flies and Beetles");
      vga.printStr(1, 18, 0x06, (char *)"You can't Run Over Yourself!");
      vga.printStr(0, 19, 0x06, (char *)"Can't go beyond Green Borders");

      vga.printStr(1, 22, 0x0C, (char *)"Joystick Button Ends Game!");
      vga.printStr(2, 23, 0x0C, (char *)"Joystick Sets Direction");
    } else timeCounter = 0;

    if ((timeCounter % 20) > 9) vga.printStr(1, 27, 0x08, (char *)"Press Any  Button to Start");
    else vga.printStr(1, 27, 0x00, (char *)"Press Any  Button to Start");
  }
}

void loop() {
// everything is within setup()
}
