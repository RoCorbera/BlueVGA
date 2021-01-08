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
#include "Joystick.h"

// using an Arduino Analog Joystick - VRX Analog Signal on PB0, VRY Analog Signal on PB1 and SW on PB10
// this video at https://www.youtube.com/watch?v=f8iaW5YcsNE shows the joystick used in this scketch
Joystick joystick(PB0, PB0, PB1, PB1, PB10);


// creates a BlueVGA object using ASCII_FONT as bitmap for all the tiles (ASCII characters)
BlueVGA vga(ASCII_FONT);

void setup() {
  // draws the inital screen
  vga.clearScreen();     // ASCII 0x20 (blank space) is, by default, used to fill the whole screen. Here we set background color to black, and also black for foreground!
  vga.printStr(1, 2, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"Move the Joystick & watch");
  vga.printStr(1, 3, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"watch analogRead changes!");
  vga.printStr(1, 7, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)"Analog Value of VRX: 000");
  vga.printStr(1, 9, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)"Analog Value of VRY: 000");
  vga.printStr(2, 11, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)"Button is pressed: yes");
}

void loop() {
  vga.waitVSync(6); // equivalent to Arduino delay(100); -- 6 * 1/60 of second

  // displays values of analog port PB0, PB1 used to connect VRX, VRY pins of the analog joystick as declared above "Joystick joystick(PB0, PB0, PB1, PB1, PB10);"
  vga.printInt(22, 9, analogRead(PB0), vga.getColorCode(RGB_MAGENTA, RGB_BLACK), true, 4);
  vga.printInt(22, 7, analogRead(PB1), vga.getColorCode(RGB_MAGENTA, RGB_BLACK), true, 4);

  // checks if Joystick has its button pressed and displays yes or no, accordingly to its state
  if (joystick.firePressed())
    vga.printStr(21, 11, vga.getColorCode(RGB_GREEN, RGB_BLACK), (char *)"yes");
  else
    vga.printStr(21, 11, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)"no ");
}

