/*
   BlueVGA library - VGA Driver Library for STM32F103

   - This library is intended to work in Arduino IDE using Bluepill STM32F103C8 or STM32F103CB boards
   - It uses ARDUINO Roger's core for STM32F103C board. Please check it at https://github.com/rogerclarkmelbourne/Arduino_STM32
     you will find arduino installation for Roger's core at https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/Installation
   - It was tested and runs using the following Arduino Settings for the board:
       Generic STM32F103C series
       Optimize Os (Smallest)
       Variant STM32F103C8 or STM32F103CB
       CPU Speed(MHz) 72MHz (Normal)

    Author Rodrigo Patricio Garcia Corbera (rocorbera@gmail.com)
    Copyright © 2017-2020 Rodrigo Patricio Garcia Corbera. 
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
#include "bluevgadriver.h"


/*

   Check bluevga.h for documentation about the functions

*/

void BlueVGA::waitVSync(uint16_t waitFrames) {
  uint32_t myNextFrame = frameNumber + waitFrames;
  while (myNextFrame >= frameNumber) asm volatile ("wfi");
}

uint32_t BlueVGA::getFrameNumber() {
  return frameNumber;
}

void BlueVGA::setBitmap(const uint8_t *bmap) {
  TBitmap = (uint8_t *)bmap;
}

void BlueVGA::setBGColor(uint8_t x, uint8_t y, uint8_t c) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  CRAM[y][x] &= 0x0F;
  CRAM[y][x] |= (c << 4);
}

uint8_t BlueVGA::getBGColor(uint8_t x, uint8_t y) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  return (CRAM[y][x] >> 4);
}

void BlueVGA::setFGColor(uint8_t x, uint8_t y, uint8_t c) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  c = c & 0x0F;
  CRAM[y][x] &= 0xF0;
  CRAM[y][x] |= c;
}

uint8_t BlueVGA::getFGColor(uint8_t x, uint8_t y) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  return (CRAM[y][x] & 0x0F);
}

uint8_t BlueVGA::getColorCode (uint8_t cfg, uint8_t cbg) {
  return ((cbg << 4) | (cfg & 0x0F));
}

uint8_t BlueVGA::getReversedColorCode (uint8_t x, uint8_t y) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  uint16_t c = CRAM[y][x];
  return ((c << 4) | (c >> 4));
}

void BlueVGA::setColor(uint8_t x, uint8_t y, uint8_t c) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  CRAM[y][x] = c;
}

void BlueVGA::setColorRegion(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t c) {
  x1 = x1 % VRAM_WIDTH;
  y1 = y1 % VRAM_HEIGHT;
  x2 = x2 % VRAM_WIDTH;
  y2 = y2 % VRAM_HEIGHT;
  if (x1 > x2) {
    uint8_t x = x2;
    x2 = x1;
    x1 = x;
  }
  if (y1 > y2) {
    uint8_t y = y2;
    y2 = y1;
    y1 = y;
  }
  for (uint8_t y = y1; y <= y2; y++)
    for (uint8_t x = x1; x <= x2; x++) {
      CRAM[y][x] = c;
    }
}

void BlueVGA::setTile(uint8_t x, uint8_t y, uint8_t t) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  TRAM[y][x] = t;
}

uint8_t BlueVGA::getTile(uint8_t x, uint8_t y) {
  x = x % VRAM_WIDTH;
  y = y % VRAM_HEIGHT;
  return (TRAM[y][x]);
}

void BlueVGA::setTile(uint8_t x, uint8_t y, uint8_t t, uint8_t fgc, uint8_t bgc) {
  setTile(x, y, t);
  setFGColor(x, y, fgc);
  setBGColor(x, y, bgc);
}

void BlueVGA::setTile(uint8_t x, uint8_t y, uint8_t t, uint8_t color) {
  setTile(x, y, t);
  setColor(x, y, color);
}

void BlueVGA::printStr(uint8_t x, uint8_t y, uint8_t color, char *str) {
  uint8_t l = strlen(str);
  if (l) {
    x = x % VRAM_WIDTH;
    y = y % VRAM_HEIGHT;
    if (l > VRAM_WIDTH - x) l = VRAM_WIDTH - x;
    for (uint8_t p = x; p <= x + l - 1; p++) {
      TRAM[y][p] = str[p - x] & 0x7F;
      CRAM[y][p] = color;
    }
  }
}

void BlueVGA::printInt (uint8_t x, uint8_t y, uint32_t number, uint8_t color, bool leadingZeros, uint8_t spaceForDigits) {
  int8_t offsetX = spaceForDigits - 1;
  bool printAtLeastZero = true;
  if (!spaceForDigits) return; // no digits to print...
  do {
    if (x + offsetX < 0 || x + offsetX >= VRAM_WIDTH) return; // clip it for screen margins
    uint8_t digit = number % 10;
    if (number > 0 || printAtLeastZero) setTile(x + offsetX, y, '0' + digit, color);
    else if (leadingZeros) setTile(x + offsetX, y, '0', color);
    else setTile(x + offsetX, y, ' ', color);
    number /= 10;
    offsetX--;
    printAtLeastZero = false;
  } while (offsetX >= 0);
}

void BlueVGA::clearScreen(uint8_t color, uint8_t tile) {
  for (uint8_t y = 0; y < VRAM_HEIGHT; y++)
    for (uint8_t x = 0; x < VRAM_WIDTH; x++)
      setTile(x, y, tile, color);  // default: ASCII 32 for space or blank  + default color black on black
}


void BlueVGA::beginVGA(const uint8_t *bmap) {
  systick_disable();
  if (bmap) setBitmap(bmap);
  else setBitmap(defaultTile);  // in case bmap is NULL, use a minimum tile bitmap of 1 default empty tile
  // default screen in blue...
  clearScreen(0x20, 0);
  video_init();
}

void BlueVGA::endVGA() {
  video_end();
}

BlueVGA::BlueVGA() {
  beginVGA();
}

BlueVGA::BlueVGA(const uint8_t *bmap) {
  beginVGA(bmap);
}

BlueVGA::~BlueVGA() {
  endVGA();
}



