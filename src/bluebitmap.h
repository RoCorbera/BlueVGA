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

#ifndef BLUEGRAPH_H
#define BLUEGRAPH_H

#define DO_NOT_PAINT_COLOR   -1

#include "bluevga.h"

// used when creating a BlueVGA object with our local RAM font as Bitmap source
#define USE_RAM BlueBitmap::ramFont

#ifdef __cplusplus
class BlueBitmap {

  private:
    uint8_t width, height;
    uint8_t *bitmap;

    // Let's create a bitmap font on RAM. 256 characters using 8 bytes each character
    // This will allow us to draw anything on this font bitmap
    // We can set a tile at any place on the screen and draw its pixels one by one

    static BlueVGA *blueVgaObject;
    static uint8_t nextFreeTile;       // Tile 0 is used as default background
    static uint8_t firstFreeTile;      // reference index of the first usable tile for drwaing pixels on the screen

  public:
    // this function dinamicaly allocates RAM tiles from ramFont when necessary and set it to the position
    // where the pixel will be set or reset
    // the control of what tiles ar already allocated is done with firstFreeTile and nextFreeTile
    // It works as a circular buffer, thus if we set too many pixels apart from each other, we can 
    // end up reusing some tiles and messing up the screen
    // There is a limit of 256 tiles, thus a total of sort of "agglutinated" 16K pixels 
    static void drawPixel(uint8_t x, uint8_t y, bool setReset = true);

    // this function uses drawPixel to ser or reset each pixel as decribed on the bitmap of any size
    // it may be too slow to draw may bitmaps or even to a very large one
    void drawBitmap(uint8_t x, uint8_t y, uint8_t frameNum = 0, bool setReset = true, int8_t color = DO_NOT_PAINT_COLOR);

    // this function draws a bitmap of 8x8 or 16x8 faster than drawBitmap()
    // but its implementation only deals specific bitmap size
    void drawBitmap8(uint8_t x, uint8_t y, uint8_t frameNum = 0, bool setReset = true, int8_t color = DO_NOT_PAINT_COLOR);

    // this function copies an 8x8 region of flashFontChar (such as a character bitmap) indexed by flashFontChar
    // into the RAM Font space at the ramFontTileNumber tile position
    static void copyFont2RamTile (uint8_t flashFontChar, const uint8_t *fontBitmap, uint8_t ramFontTileNumber);

    // sets 0 to all RAM tiles starting on firstFreeTile and resets nextFreeTile to it
    // in a graphic context, we may want to keep some tiles fixed for some purpose
    // only tiles after firstFreeTile are used to set/reset pixels
    // by this we can split RAM tile space into two groups
    // important note is that usually tile 0 is used to fill al the screen with a pattern
    // that usually is a blank character such as ' '
    static void eraseRamTiles();
    static void clearGraphScreen(uint8_t color = ((RGB_YELLOW << 4) | RGB_BLUE));  // Yellow over blue background

    // simple constructors, no desctructor needed
    BlueBitmap(uint8_t w, uint8_t h, uint8_t *b) : width(w), height(h), bitmap(b) {};
    BlueBitmap() {};
    void setBitmap(uint8_t w, uint8_t h, const uint8_t *b) {
      width = w;
      height = h;
      bitmap = (uint8_t *)b;
    }
    
    // static functions for setting BlueVGA driver object and RAM Tile splitting settting
    static void setFirstTile(uint8_t tile) { firstFreeTile = tile; }
    static uint8_t getFirstTile() { return firstFreeTile; }
    static void setNextFreeTile(uint8_t tile) { nextFreeTile = tile; }
    static uint8_t getNextFreeTile() { return nextFreeTile; }
    static uint8_t ramFont[256 * 8];   // RAM Font Bitmap used to draw pixels
};
#endif 


#endif
