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

#ifndef BLUE_VGA_H
#define BLUE_VGA_H

#define VRAM_WIDTH            28           // Screen is composed by 28x30 characters or tiles of 8x8 pixels each
#define VRAM_HEIGHT           30           // Thus it is designed to displaying those tiles in any place in the screen of 28x30 
#define TILE_W                8            // Tile Width
#define TILE_H                8            // Tile Height

// constants for each of the 8 possible colors. It uses RGB0 (4 bits) for each color - but we use only 3 pins (PC15-13), with 3bpp or 8 colors
typedef enum {
  RGB_RED     = 0x8,
  RGB_GREEN   = 0x4,
  RGB_BLUE    = 0x2,
  RGB_YELLOW  = 0xC,
  RGB_MAGENTA = 0xA,
  RGB_CYAN    = 0x6,
  RGB_WHITE   = 0xE,
  RGB_BLACK   = 0x0,
} rgbColors;


#ifdef __cplusplus
class BlueVGA {

  private:
    // empty pattern on tile zero as default for case no bitmap is declared and initialized (avoid NULL pointer issue)
    const uint8_t defaultTile[8] = {0};

  public:

    // constructors and destructor
    BlueVGA(const uint8_t *bmap);
    BlueVGA();
    ~BlueVGA();

    /*
       This lib works on Color VGA 640x480 @ 60Hz. It draws 60 Frames per Second.
       In order to make the screen looks solid and animations look smooth, use waitVSync to hold the execution up to the end of VGA Frame drawing.
       This means you should execute the scketch in the VBLANK time space for better results.
       waitVSync can hold execution by a number of Frames, allowing to use it as an alternative for Arduino delay(). It delays 1/60 of a second.
    */
    void waitVSync(uint16_t waitFrames = 1);
    uint32_t getFrameNumber();                  // it returns a Frame Sequenced Number

    // allows to set the bitmap used to draw tiles in the screen...
    void setBitmap(const uint8_t *bmap);

    /*
       The display has 28x30 tiles that can use 2 colors each. Foreground color for pixels "1" ans background color for pixels "0"
       Tiles are 8x8 bitmaps coded from 0 to 255. Thus it can olny display up to 255 different tile partterns.
       Each background and foreground color is 3 bpp (bits per pixel) and is structures as RGB0 - 1 bits for Red, 1 for Green and 1 for Blue and a last bit zero
       It's possible to use all those 8 color in the screen at the same time.
       Painting BG and FG colors are done at x,y related to a 28x30 coordinate tile system.
    */
    void setFGColor(uint8_t x, uint8_t y, uint8_t cfg);   // paints foreground color of tile at x,y with color C in format RGB0 (4 bits)
    uint8_t getFGColor(uint8_t x, uint8_t y);             // gets the foreground color of tile at x,y
    void setBGColor(uint8_t x, uint8_t y, uint8_t cbg);   // paints background color of tile at x,y with color C in format RGB0 (4 bits)
    uint8_t getBGColor(uint8_t x, uint8_t y);             // gets the background color of tile at x,y

    uint8_t getColorCode (uint8_t cfg, uint8_t cbg);      // helper function for returning a single 8 bits color that describes Back and Foreground color of a Tile
    uint8_t getReversedColorCode (uint8_t x, uint8_t y);  // helper function for returning a single 8 bits color that swaps Back and Foreground color of a Tile
    void setColor(uint8_t x, uint8_t y, uint8_t color);   // helper function for setting a Back and Foreground color of a Tile with a single 8 bits color
    void setColorRegion(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color); // helper function for setting the BG+FG color of a region of the screen

    /*
       The display has 28x30 tiles that can use 2 colors each. Foreground color for pixels "1" ans background color for pixels "0"
       Tiles are 8x8 bitmaps coded from 0 to 255. Thus it can olny display up to 255 different tile partterns.
       Positioning of a tile is related to a 28x30 coordinates of the screen!
    */
    void setTile(uint8_t x, uint8_t y, uint8_t tile);                            // works as a printChar, placing a character or Tile at x,y - does note modify tile color
    void setTile(uint8_t x, uint8_t y, uint8_t t, uint8_t color);                // Helper function for placing a tile at xxy using color 4+4 bits for BG+FG color in single 8bits code
    void setTile(uint8_t x, uint8_t y, uint8_t t, uint8_t fgc, uint8_t bgc);     // Helper function for placing a tile at x,y using colors as foreground and background RGB0 4 bits each
    uint8_t getTile(uint8_t x, uint8_t y);                                       // returns the Tile at x,y in the screen VRAM (video RAM)

    /*
        simple helper function that displays a string or any NULL terminated tile sequence in the screen.
        In order to use it, the scketch mus set tile Bitmaps first.
        By setting tile for ASCII codes 0 to 127, printString will work as a plain print function.
        It does NOT understand any escape code such as \t or \n!
    */
    void printStr(uint8_t x, uint8_t y, uint8_t color, char *str);
    // prints a integer using color, with/without leading '0's, limited to <spaceForDigits> digits/tiles
    void printInt (uint8_t x, uint8_t y, uint32_t number, uint8_t color, bool leadingZeros = false, uint8_t spaceForDigits = 5);

    // Helper functions for clearing the screen by copying a ' ' (blank character ASCII code 0x20) on every place of the screen
    void clearScreen(uint8_t color = 0, uint8_t tile = ' ');

    /*
        beginVGA starts VGA sginaling of Bluepill
        it uses pins PA9 as Horizontal Sync VGA signal
                     PB6 as Vertical Sync VGA signal
                     PC13 for Blue VGA signal
                     PC14 for Green VGA signal
                     PC15 for Red VGA signal
         It also uses TIM1 and TIM4 of STM32F103C[8B]T6 runing at 72Mhz

         IMPORTANT NOTICE: this Library deactivates Systick, thus delay(), micros(), millis(), delayMicroseconds() will not work!
                           in order to delay the execution, use waitVSync(time in 1/60 of second)
                           or use getFrameNumber() to know number of Frames since sketch started execution in 1/60 second units ==> 16.66 milliseconds

    */
    void beginVGA(const uint8_t *bmap = NULL);
    void endVGA();

};
#endif

#endif

