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
    Copyright � 2017-2021 Rodrigo Patricio Garcia Corbera. 
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


#ifndef VGA_SIZE_H
#define VGA_SIZE_H

#define VRAM_WIDTH            28           // Screen is composed by 28x30 characters or tiles of 8x8 pixels each
#define VRAM_HEIGHT           30           // Thus it is designed to displaying those tiles in any place in the screen of 28x30 
#define TILE_W                8            // Tile Width
#define TILE_H                8            // Tile Height

// constants for each of the 8 possible colors. It uses RGB0 (4 bits) for each color - but we use only 3 pins (PC15-13), with 3bpp or 8 colors
typedef enum {
  RGB_BLACK   = 0x0,
  RGB_BLUE    = 0x2,
  RGB_GREEN   = 0x4,
  RGB_CYAN    = 0x6,
  RGB_RED     = 0x8,
  RGB_MAGENTA = 0xA,
  RGB_YELLOW  = 0xC,
  RGB_WHITE   = 0xE,
} rgbColors;

#endif
