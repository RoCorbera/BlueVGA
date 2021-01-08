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
    Redistributions of source code must retain the above copyright notice, and meet all conditions as defined in  https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the this disclaimer in 
    the documentation and/or other materials provided with the distribution.

    ** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. 
    ** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
    ** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

#ifndef BLUE_VGA_H
#define BLUE_VGA_H

#ifdef __cplusplus
#include "vgaProperties.h"
#include "Print.h"

class BlueVGA : public Print{

  private:
    // empty pattern on tile zero as default for case no bitmap is declared and initialized (avoid NULL pointer issue)
    const uint8_t defaultTile[8] = {0};
    // Using Print Class from Arduino. This allows to use BlueVGA::print(...) and BlueVGA::println(...)
    uint8_t cursorX = 0, cursorY = 0; // cursor position for printing (28x30)
    bool wrap = true;                 // define if print beyond right screen margin will coninue on next line
    uint8_t fgColor = RGB_YELLOW, bgColor = RGB_BLUE;          // colors for foreground and cackground when printing
    uint8_t textTabSize = 4;         // default set TAB in 4 spaces
    
  public:
#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
    size_t write(uint8_t ch);
    size_t write(const uint8_t *buffer, size_t size);
#endif    
    
#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
    size_t write(uint8_t ch);            // implementing virtual function write() to implement Print Class
    size_t write(const char *str);
    size_t write(const void *buf, uint32_t len);    
#endif    

    // constructors and destructor
    BlueVGA(const uint8_t *bmap);
    BlueVGA();
    ~BlueVGA();

    /*
        Functions to manipulate text printing in conjunction with Print Class
        For printing on screen it's possible to use print() and println()
        Text will be scrolled up if last line of the screen is printed with println()
        Text has a foreground color and a bakcground color for each character
        Printing at the en of the screen line can continue on next line or be wrapped at its edge
        
    */
    inline void setTextColor(uint8_t cfg = RGB_WHITE) { 
        fgColor = cfg; 
    }
    inline void setTextColor(uint8_t cfg = RGB_WHITE, uint8_t cbg = RGB_BLUE) { 
        fgColor = cfg; 
        bgColor = cbg; 
    }
    inline uint8_t getTextColor() { 
        return ((bgColor << 4) | (fgColor & 0x0F));
    }
    inline void setTextCursor(uint8_t x = 0, uint8_t y = 0) { 
        cursorX = x < VRAM_WIDTH ? x: VRAM_WIDTH; 
        cursorY = y < VRAM_HEIGHT ? y : VRAM_HEIGHT; 
    } 
    inline void setTextWrap(bool w = true) { 
        wrap = w; 
    }
    inline uint8_t getTextCursorX() { 
        return(cursorX);
    } 
    inline uint8_t getTextCursorY() { 
        return(cursorY);
    } 
    inline void setTextTab(uint8_t t = 4) { 
        textTabSize = t; 
    }

    void scrollText(uint8_t lines = 1);


    /*
       This lib works on Color VGA 640x480 @ 60Hz. It draws 60 Frames per Second.
       In order to make the screen looks solid and animations look smooth, use waitVSync to hold the execution up to the end of VGA Frame drawing.
       This means you should execute the scketch in the VBLANK time space for better results.
       waitVSync can hold execution by a number of Frames, allowing to use it as an alternative for Arduino delay(). It delays 1/60 of a second.
    */
    void waitVSync(uint16_t waitFrames = 1);
    uint32_t getFrameNumber();                  // it returns a Frame Sequenced Number

    // allows to set the bitmap used to draw tiles in the screen...
    void setFontBitmap(const uint8_t *bmap);

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
    void setTileRowsFast(uint8_t y1 = 0, uint8_t y2 = VRAM_HEIGHT - 1, uint8_t tile = 0);
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

    // Helper function to fill the whole screen with a specific tile, mostly used in conjuntion with BlueGraph Class
    void fillScreen(uint8_t tile = 0);

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

