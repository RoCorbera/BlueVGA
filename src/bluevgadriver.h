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

#ifndef BLUE_VGA_DRIVER_H
#define BLUE_VGA_DRIVER_H

#include "vgaProperties.h"

#ifdef __cplusplus
extern "C" {
#endif
void video_init(uint8_t flashFont);
void video_end(void);
// using VRAM_HEIGHT + 1 to make possible a simple way for scrolling text and graphics
extern uint8_t TRAM [VRAM_HEIGHT][VRAM_WIDTH];  // VRAM with Characters or Tiles of 8 x 8 pixels
extern uint8_t CRAM [VRAM_HEIGHT][VRAM_WIDTH];  // Color VRAM - color = 3bits + 3bits (Foreground + Background) - 8 colors on PC15-PC13 port (RGB)
extern uint8_t *TBitmap;
extern volatile uint32_t frameNumber;
#ifdef __cplusplus
}
#endif

#endif


