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
#include "bluevga.h"
#include "bluevgadriver.h"


/*

   Check bluevga.h for documentation about these methods/functions

*/

void BlueVGA::waitVSync(uint16_t waitFrames) {
  uint32_t myNextFrame = frameNumber + waitFrames;
  while (myNextFrame > frameNumber) asm volatile ("wfi");
}

uint32_t BlueVGA::getFrameNumber() {
  return frameNumber;
}


void BlueVGA::setFontBitmap(const uint8_t *bmap) {
  bool flashFont = ((uint32_t) bmap) < 0x20000000;
  if (bmap) {
    TBitmap = (uint8_t *)bmap;
    // allow to exchange between Flash Tile Bitmap and RAM Tile Bitmap 
#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
    TIM1->CCR1 = ((uint32_t) bmap) < 0x20000000 ? 40 : 165;
#endif
#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
    TIMER1_BASE->CCR1 = ((uint32_t) bmap) < 0x20000000 ? 10 : 135;
#endif
  }
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

void BlueVGA::setTileRowsFast(uint8_t y1, uint8_t y2, uint8_t tile) {
  if (y1 > y2 || y1 >= VRAM_HEIGHT || y2 >= VRAM_HEIGHT) return;

  uint32_t fourTiles = tile | tile << 8 | tile << 16 | tile << 24;
  uint32_t *rowHead = (uint32_t *) TRAM[y1];
  for (uint8_t i = y1; i <= y2; i++) {
    // 4 * 7 = 28 tiles per row
    *rowHead++ = fourTiles;
    *rowHead++ = fourTiles;
    *rowHead++ = fourTiles;
    *rowHead++ = fourTiles;
    *rowHead++ = fourTiles;
    *rowHead++ = fourTiles;
    *rowHead++ = fourTiles;
  }
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


// set all tiles of the screen to a specific tile number, not changing screen colors
void BlueVGA::fillScreen(uint8_t tile) {
  setTileRowsFast(0, VRAM_HEIGHT - 1, tile);
}


void BlueVGA::clearScreen(uint8_t color, uint8_t tile) {
  for (uint8_t y = 0; y < VRAM_HEIGHT; y++)
    for (uint8_t x = 0; x < VRAM_WIDTH; x++)
      setTile(x, y, tile, color);  // default: ASCII 32 for space or blank  + default color black on black
}


void BlueVGA::beginVGA(const uint8_t *bmap) {
#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
  systick_disable();
#endif
#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
  SysTick->CTRL = 0;    //Disable Systick
#endif

  // default screen in blue...
  clearScreen(0x20, 0);
  if (bmap) setFontBitmap(bmap);
  else setFontBitmap(defaultTile);  // in case bmap is NULL, use a minimum tile bitmap of 1 default empty tile
  video_init(((uint32_t) bmap) < 0x20000000);
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


// Vertical Scroll manipulation
void BlueVGA::scrollText(uint8_t lines) {
  if (!lines) return;
  lines %= VRAM_HEIGHT;
  uint32_t *srcT = (uint32_t *)TRAM[lines], *srcC = (uint32_t *)CRAM[lines];  // 32bits at once
  uint32_t *dstT = (uint32_t *)TRAM[0], *dstC = (uint32_t *)CRAM[0];          // 32bits at once
  for (uint8_t y = lines; y < VRAM_HEIGHT; y++) {
    // it's faster not using loops
    *dstT++ = *srcT++; *dstT++ = *srcT++; *dstT++ = *srcT++; *dstT++ = *srcT++;
    *dstT++ = *srcT++; *dstT++ = *srcT++; *dstT++ = *srcT++;
    *dstC++ = *srcC++; *dstC++ = *srcC++; *dstC++ = *srcC++; *dstC++ = *srcC++;
    *dstC++ = *srcC++; *dstC++ = *srcC++; *dstC++ = *srcC++;
  }
//  uint8_t dstColor1 = ((bgColor << 4) | (fgColor & 0x0F));
//  uint32_t dstColor4 = dstColor1;                                             // 32bits at once
//  dstColor4 = (dstColor4 << 8) | dstColor1;
//  dstColor4 = (dstColor4 << 8) | dstColor1;
//  dstColor4 = (dstColor4 << 8) | dstColor1;
  for (uint8_t y = 0; y < lines; y++) {                                         // fill last line with ' ' (space char) not modifying previous fg/bg colors
    *dstT++ = 0x20202020; *dstT++ = 0x20202020; *dstT++ = 0x20202020; *dstT++ = 0x20202020;
    *dstT++ = 0x20202020; *dstT++ = 0x20202020; *dstT++ = 0x20202020;
//    *dstC++ = dstColor4;  *dstC++ = dstColor4;  *dstC++ = dstColor4;  *dstC++ = dstColor4;
//    *dstC++ = dstColor4;  *dstC++ = dstColor4;  *dstC++ = dstColor4;
  }
}


/*

  For reference, the functions below are used to set color of the text to be printed and if it gets wrapped to the next line or not
  Everything is declared in bluevga.h

    void setTextColor(uint8_t cfg = RGB_WHITE) { 
        fgColor = cfg; 
    }
    
    void setTextColor(uint8_t cfg = RGB_WHITE, uint8_t cbg = RGB_BLUE) { 
        fgColor = cfg; 
        bgColor = cbg; 
    }
    
    uint8_t getTextColor() { 
        return ((bgColor << 4) | (fgColor & 0x0F));
    }
    
    void setTextCursor(uint8_t x = 0, uint8_t y = 0) { 
        cursorX = x < VRAM_WIDTH ? x: VRAM_WIDTH; 
        cursorY = y < VRAM_HEIGHT ? y : VRAM_HEIGHT; 
    }
     
    void setTextWrap(bool w = true) { 
        wrap = w; 
    }
    
    uint8_t getTextCursorX() { 
        return(cursorX);
    }
     
    uint8_t getTextCursorY() { 
        return(cursorY);
    }
     
    void setTextTab(uint8_t t = 4) { 
        textTabSize = t; 
    }

*/

// These functions enable Arduino print() and println() to work in the same way it does with Serial.print/println()
size_t BlueVGA::write(uint8_t ch) {

  if (cursorY == VRAM_HEIGHT) {      // does it overflow screen bottom?
    cursorY = VRAM_HEIGHT - 1;       // keep cursorY at last line of the screen
    scrollText();                    // scroll text up 1 line
  }
  if (ch == '\r') cursorX = 0;           // places cursor at the begining of the line
  else {
    // prints the character in the screen and updates cursor position -- only ASCII printable characters
    if (cursorX < VRAM_WIDTH && ch > 31 && ch < 127)
      setTile(cursorX++, cursorY, ch, fgColor, bgColor);
    if (ch == '\t')                      // TAB has 4 spaces spaces and it aligns to the next X that is multiple of 4
      cursorX = ((cursorX + textTabSize) / textTabSize) * textTabSize;
    if (ch == '\n' || (cursorX >= VRAM_WIDTH && wrap)) { // check wrapping
      cursorX = 0;
      cursorY++;
    }
  }
  return 1;
}


#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
size_t BlueVGA::write(const uint8_t *buf, size_t len)
#endif
#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32  
size_t BlueVGA::write(const void *buf, uint32_t len)
#endif
{
  uint8_t *strBuf = (uint8_t *) buf;
  size_t n = 0;
  if (len) for (uint16_t i = 0; i < len; i++) {
      size_t ret = write(strBuf[i]);
      if (!ret) break;
      n += ret;
    }
  return n;
}

#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32  
size_t BlueVGA::write(const char *str) {
  if (str) return (write((const void *)str, strlen(str)));
  else return 0;
}
#endif


