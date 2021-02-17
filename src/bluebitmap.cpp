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

#include "Arduino.h"
#include "bluevgadriver.h"
#include "bluebitmap.h"

uint8_t BlueBitmap::nextFreeTile = 1;       // Tile 0 is used as default background
uint8_t BlueBitmap::firstFreeTile = 1;      // reference index of the first usable tile for drwaing pixels on the screen
uint8_t BlueBitmap::ramFont[256 * 8] = {0}; // RAM Font Bitmap used to draw pixels


void BlueBitmap::drawPixel(uint8_t x, uint8_t y, bool setReset) {
  // clip x and y to the limits of the screen, just in case...
  x %= (VRAM_WIDTH << 3);
  y %= (VRAM_HEIGHT << 3);

  // get the tile position in the screen
  uint8_t xTile = (x >> 3);
  uint8_t yTile = (y >> 3);

  // get the pixel postion in the tile
  uint8_t xPos = x & 7;
  uint8_t yPos = y & 7;

  // is the Tile we will draw already used for drawing pixel or we must alocate a new tile from the pool?
  uint8_t tileIdx = TRAM[yTile][xTile];
  if (!tileIdx) {     // we must alocate a free tile to draw the pixel
    tileIdx = nextFreeTile++;
    // set the allocated tile in the screen
    TRAM[yTile][xTile] = tileIdx;
    // if we used all possible free tiles, we just restart it... is it good?
    if (!nextFreeTile) nextFreeTile = firstFreeTile;
  }

  // set the mask for AND / OR operation on the pixel (bit) we want to draw
  uint8_t mask = setReset ? (1 << (7 - xPos)) : ~ (1 << (7 - xPos));

  // find the right byte on ramFont bitmap and set or reset the pixel
  uint8_t *tileY = ramFont + (tileIdx << 3) + yPos;
  if (setReset) *tileY |= mask;
  else *tileY &= mask;
}

// just erase memory using 32bits at once -> faster
void BlueBitmap::eraseRamTiles() {
  uint32_t *ramTile32Bits = (uint32_t *) (ramFont + (firstFreeTile << 3));
  for (uint8_t i = firstFreeTile; i < nextFreeTile; i++) {
    *ramTile32Bits++ = 0;
    *ramTile32Bits++ = 0;
  }
  nextFreeTile = firstFreeTile;
}


void BlueBitmap::clearGraphScreen(uint8_t color) {
  // reset RAM Tiles pool and erase them
  BlueBitmap::eraseRamTiles();
  // Fill the screen with tile ZERO - necessary to get the bitmap system working
  uint32_t *TRAM32Bits = (uint32_t *) TRAM;
  // set tile color in the screen
  uint32_t *CRAM32Bits = (uint32_t *) CRAM;
  uint32_t color32Bits = color << 24 | color << 16 | color << 8 | color;
  // faster using 32 bits operations
  for (uint8_t y = 0; y < VRAM_HEIGHT; y++) {
    // 4 x 7 = 28 tiles
    *TRAM32Bits++ = 0; *TRAM32Bits++ = 0; *TRAM32Bits++ = 0; *TRAM32Bits++ = 0; 
    *TRAM32Bits++ = 0; *TRAM32Bits++ = 0; *TRAM32Bits++ = 0;
    // 4 x 7 = 28 Colors
    *CRAM32Bits++ = color32Bits; *CRAM32Bits++ = color32Bits; *CRAM32Bits++ = color32Bits; 
    *CRAM32Bits++ = color32Bits; *CRAM32Bits++ = color32Bits; *CRAM32Bits++ = color32Bits; 
    *CRAM32Bits++ = color32Bits;
  }
}

void BlueBitmap::copyFont2RamTile (uint8_t flashFontChar, const uint8_t *fontBitmap, uint8_t ramFontTileNumber) {
  if (!fontBitmap) return;

  // faster using 32 bits operations
  uint32_t *ramTile32Bits = (uint32_t *) (ramFont + (ramFontTileNumber << 3));
  uint32_t *fontBitmap32Bits = (uint32_t *) (fontBitmap + (flashFontChar << 3));
  *ramTile32Bits++ = *fontBitmap32Bits++;
  *ramTile32Bits++ = *fontBitmap32Bits++;
}


// color must be one of those: RGB_WHITE, RGB_BLACK, RGB_RED, RGB_GREEN, RGB_BLUE, RGB_CYAN, RGB_MAGENTA, RGB_YELLOW
// or just call it with DO_NOT_PAINT_COLOR as color to do not set color and only set or reset pixels
// calling it with no color argument also means that we just want to set pixels nad do not use colors
// IMPORTANT: be aware that setting a black pixel, for instance, over a black backgroud will not produce any visual effect
// Handles any size of Bitmap to draw, but it is slow and CPU intensive
void BlueBitmap::drawBitmap(uint8_t x, uint8_t y, uint8_t frameNum, bool setReset, int8_t color) {
  if (!bitmap || !width || !height) return;
  // clip x and y to the limits of the screen, just in case...
  x %= (VRAM_WIDTH << 3);
  y %= (VRAM_HEIGHT << 3);


  // Bitmap is described in bits (pixels) from left to all way to rightest pixel, line by line
  uint8_t *bitmapFrame =  bitmap + (width & 7 ? width / 8 + 1 : width / 8) * height * frameNum;
  for (uint8_t sy = 0; sy < height; sy++)
    for (uint8_t sx = 0; sx < width; sx++) {
      uint8_t bytePixels = bitmapFrame[(((width - 1) >> 3) + 1) * sy + (sx >> 3)];
      uint8_t pixel = bytePixels & (0x80 >> (sx & 7));
      if (pixel) drawPixel(x + sx, y + sy, setReset);
    }

  if (((uint8_t)color) < 16) {   // skip this if color = -1 (DO_NOT_PAINT_COLOR)
    // get the tile position in the screen
    uint8_t xTile = (x >> 3);
    uint8_t yTile = (y >> 3);
    for (uint8_t y = yTile; y < yTile + (height >> 3) + 1; y++)
      for (uint8_t x = xTile; x < xTile + (width >> 3) + 1; x++)
        CRAM[y][x] = color;
  }
}


// color must be one of those: RGB_WHITE, RGB_BLACK, RGB_RED, RGB_GREEN, RGB_BLUE, RGB_CYAN, RGB_MAGENTA, RGB_YELLOW
// or just call it with DO_NOT_PAINT_COLOR as color to do not set color and only set or reset pixels
// calling it with no color argument also means that we just want to set pixels nad do not use colors
// IMPORTANT: be aware that setting a black pixel, for instance, over a black backgroud will not produce any visual effect
// Very special case of Bitmap here. ONLY 8x8 or 16x8 bitmaps, but very FAST!
void BlueBitmap::drawBitmap8(uint8_t x, uint8_t y, uint8_t frameNum, bool setReset, int8_t color) {
  if (!bitmap || !width || !height) return;

  // clip x and y to the limits of the screen, just in case...
  x %= (VRAM_WIDTH << 3);
  y %= (VRAM_HEIGHT << 3);

  // get the tile position in the screen
  uint8_t xTile = (x >> 3);
  uint8_t yTile = (y >> 3);

  // get the pixel postion in the tile
  uint8_t xPos = x & 7;
  uint8_t yPos = y & 7;

  uint8_t *bitmapFrame =  bitmap + (width & 7 ? width / 8 + 1 : width / 8) * height * frameNum;
  uint32_t mask[8] = {0};     // 8 rows that will be drawn in the RAM Tiles
  for (uint8_t ym = 0; ym < 8; ym++) { // Bitmap is maximum 8 rows high
    mask[ym] = ( ( width > 8 ?  *(bitmapFrame + ym * 2) | *(bitmapFrame + ym * 2 + 1) << 8 : * (bitmapFrame + ym) ) << ((width > 8 ? 16 : 24) - xPos));
  }

  uint8_t tilesW = 1 + (width > 8 ? 1 : 0) + (xPos ? 1 : 0);   // horizontal tiles that we will allocate in the screen - max Bitmap is 16 bits!
  uint8_t tilesH = 1 + (yPos ? 1 : 0);                         // vertical tiles that we will allocate in the screen - max height if 8 pixels

  // set RAM tiles on the screen - supported bitmaps are 8x8 or 16x8
  for (uint8_t th = 0; th < tilesH; th++) {    // tilesH is 0 for vertical alignment or 1 when not alligned
    uint8_t yPosAux = th ? 0 : yPos;           // upper tile? shall we start on yPos or 0?
    for (uint8_t tw = 0; tw < tilesW; tw++) {  // tilesW will be 1, 2 ou 3...
      // is the tile at the position equal to zero? We must replace it with a new RAM Tile from the pool
      uint8_t tileIdx = TRAM[yTile + th][xTile + tw];
      if (tileIdx == 0) {
        tileIdx = nextFreeTile++;
        TRAM[yTile + th][xTile + tw] = tileIdx;
        if (!nextFreeTile) nextFreeTile = firstFreeTile;
      }

      if (((uint8_t)color) < 16) {   // skip this if color = -1 (DO_NOT_PAINT_COLOR)
        CRAM[yTile + th][xTile + tw] = color;
      }

      // find the right byte on ramFont bitmap and set or reset the pixel
      uint8_t *tileY = ramFont + (tileIdx << 3) + yPosAux;
      // draw rows on the RAM Tile
      uint8_t lines = th ? yPos : 8 - yPos;
      for (uint8_t yp = 0; yp < lines; yp++) {
        uint8_t mask8 = (uint8_t)( (*(mask + yp + (th ? 8 - yPos : 0))) >> ((3 - tw) << 3) );
        if (setReset) *tileY++ |= mask8;
        else *tileY++ &= ~mask8;
      }
    }
  }

}
