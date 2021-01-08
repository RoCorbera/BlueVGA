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

// creates a BlueVGA object using ASCII_FONT as bitmap for all the tiles (ASCII characters)
BlueVGA vga(ASCII_FONT);

void setup() {
  ScreenSetup();    // performs initial screen character drawing
  Animation();      // forever loop...
}


void loop() {
  // no need to code anything here... it's just a function that is called for ever
  // setup() can do it all, with a for_ever_loop such as in Animation()
}


/*
    Performs a simple color animation
*/
void Animation (void) {

  static uint8_t lastLineColor = 1;
  // defines an order and sequence of colors for the animation
  const uint8_t colors[8] = {RGB_RED, RGB_YELLOW, RGB_GREEN, RGB_CYAN, RGB_BLUE, RGB_MAGENTA, RGB_BLACK, RGB_WHITE};

  while (true) {   // forever ... sort of replaces loop()

    // the animation is excecuted 2 times per second...
    vga.waitVSync(30); // blocks the execution until 30 frames are past. At 60 FPS (frames per second) 30/60s = 500 milliseconds

    // gets the current Foreground and Backgorund colors for a specific position in screen
    // in particular, this position is the top left cornner of the ASCII chart at screen
    
    uint8_t x = (VRAM_WIDTH - 16) / 2;
    uint8_t fc = vga.getFGColor(x, 3);
    uint8_t bc = vga.getBGColor(x, 3);

    // rotates foreground and background colors.
    bc += 2; // colors are 4 bits, but only 3 most significative bit are used, thus it is always a pair number
    // Background goes from 0 to 14 and then foreground changes
    if (bc > 14) { // it goes up to 14 (0xE) = white
      bc = 0;
      fc += 2;
    }
    if (fc > 14) fc = 0;

    // changes ASCII chart colors
    vga.setColorRegion(x, 3, x + 16 - 1, 8, vga.getColorCode(fc, bc));

    // prints "last line" at the bottom of the screen and rotates its color as defined in the array colors[]
    lastLineColor = ++lastLineColor & 7; // increments and keeps the range in 0..7 for a single color index in the sequence we defined
    x = (VRAM_WIDTH - 13) / 2;
    vga.setColorRegion(x, 29, x + 13, 29, vga.getColorCode(colors[lastLineColor], 0)); // foreground, background colors

    // prints a number (color code) with leading '0's and 2 digits
    vga.printInt (24, 29, colors[lastLineColor], vga.getColorCode(RGB_BLACK, RGB_WHITE), true, 2);
  }
}

void ScreenSetup (void) {

  vga.clearScreen();
  vga.printStr(0, 20, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"12345678901234567890123456789012");

  for (uint8_t i = 0; i < VRAM_WIDTH; i++) {  // draws character #127 (checker) in 3 rows in the screen, alternating colors, keeping Red as main color
    // there are 3 ways for drawing a single character in the screen, using setTile(...):
    vga.setTile(i, 22, 127, vga.getColorCode(RGB_RED, RGB_YELLOW));   // using setTile(...) with a color at last paramenter
    vga.setTile(i, 23, 127, RGB_BLACK, RGB_RED);                      // using setTile(...) with arguments separated for foreground and background colors
    vga.setTile(i, 24, 127);                                          // using setTile(...) with no color argument, thus it doesn't change the colors of that x,y position
    vga.setColor(i, 24, vga.getColorCode(RGB_RED, RGB_WHITE));        // then it may be possible to only change the color at x,y not changing the tile at that position
  }

#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
  vga.printStr((VRAM_WIDTH - 18) / 2, 0, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"Roger's Core DEMO!");
#endif

#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
  vga.printStr((VRAM_WIDTH - 16) / 2, 0, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"STM32 Core DEMO!");
#endif

  // prints all possible colors using text in the screen
  uint8_t xCenter = (VRAM_WIDTH - 14) / 2;
  vga.printStr(xCenter, 11, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)    "-----RED------");
  vga.printStr(xCenter, 12, vga.getColorCode(RGB_MAGENTA, RGB_BLACK), (char *)"---MAGENTA----");
  vga.printStr(xCenter, 13, vga.getColorCode(RGB_BLUE, RGB_BLACK), (char *)   "-----BLUE-----");
  vga.printStr(xCenter, 14, vga.getColorCode(RGB_CYAN, RGB_BLACK), (char *)   "-----CYAN-----");
  vga.printStr(xCenter, 15, vga.getColorCode(RGB_GREEN, RGB_BLACK), (char *)  "----GREEN-----");
  vga.printStr(xCenter, 16, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *) "----YELLOW----");
  vga.printStr(xCenter, 17, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)  "----WHITE-----");
  vga.printStr(xCenter, 18, vga.getColorCode(RGB_BLACK, RGB_WHITE), (char *)  "BLACK REVERSED");
  vga.printStr(xCenter, 29, vga.getColorCode(1, RGB_BLACK), (char *)  "--Last Line--");

  // prints the basic ASCII table in the screen
  for (uint8_t y = 3; y < 9; y++)
    for (uint8_t x = (VRAM_WIDTH - 16) / 2; x < (VRAM_WIDTH - 16) / 2 + 16; x++) {
      vga.setTile(x, y, ' ' - (VRAM_WIDTH - 16) / 2 + x + (y - 3) * 16);
      vga.setColor(x, y, vga.getColorCode(RGB_WHITE, RGB_BLACK));
    }
}


