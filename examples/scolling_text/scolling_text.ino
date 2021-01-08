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

#include "bluevga.h"
#include "font.h"

// creates a BlueVGA object using ASCII_FONT as bitmap for all the tiles
BlueVGA vga(ASCII_FONT);


void setup() {
  // VGA already started by the constructor of BlueVGA object ... nothing to be done!
}

void loop() {
  const char hello[] = "Hello World!";
  const uint8_t textLen = strlen(hello);
  static uint8_t textX = 0;

  // the animation is excecuted 10 times per second...
  vga.waitVSync(6); // blocks the execution until 6 frames are past. At 60 FPS (frames per second) 6/60s = 100 milliseconds

  vga.clearScreen(vga.getColorCode(RGB_YELLOW, RGB_BLUE));                        // FG Color = Yellow || BG Color = Blue using 3 bits per color in format RGB0 (4 bits with a bit 0 = 0)
  textX = ++textX % VRAM_WIDTH;                                                   // moves text one position to the right and keeps X position in range (0 .. VARAM_WIDTH-1)
  vga.printStr(textX, 5, vga.getColorCode(RGB_YELLOW, RGB_BLUE), (char *)hello);  // printStr() clips the string at rigth border
  if (VRAM_WIDTH - textX < textLen)                                               // prints clipped text chunk in the left side of the screen
    vga.printStr(0, 5, vga.getColorCode(RGB_YELLOW, RGB_BLUE), (char *) hello + VRAM_WIDTH - textX);
}

