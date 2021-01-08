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

// create a BlueVGA object using ASCII_FONT as bitmap for all the tiles (ASCII characters)
BlueVGA vga(ASCII_FONT);

void setup() {
  ScreenSetup();        // performs initial screen caracter drawing
  vga.waitVSync(60*3);  // same as delay(3000) - waits 60 VGA frames * 3 -- in 60 VGA frames per second
  Animation();          // forever loop...
}


void loop() {
  // no need to code anything here... it's just a function that is called for ever
  // setup() can do it all, with a for_ever_loop such as in Animation()
}


/*
    Performs a simple color animation +
*/
void Animation (void) {

  static uint8_t lastLineNumber = 0, lineNumber = 0;

  while (true) {   // forever ... sort of replaces loop()

    // blocks the execution until 4 frames are past. 
    vga.waitVSync(4); // At 60 FPS (frames per second) 4/60s = 67 milliseconds

    // TESTING CODE.... uncomment it to use it
    //static float x = 3.14156592, inc = 5;
    //vga.setTextTab(5);
    //vga.setTextWrap(false);
    //vga.print(x,4);
    //vga.print(' ');
    //vga.print(127, HEX);
    //vga.print("\tQQ ");
    //x += inc; inc += 5;

    
    uint8_t currentCursorColor = vga.getTextColor();
    uint8_t fc = currentCursorColor & 0xF;   // color 8 bits format: rgb0rgb0 BG-FG 4 bits each
    // rotates foreground and backgorund colors.
    fc += 2; // colors are 4 bits, but only 3 most significative bit are used, thus it is always a pair number
    // foreground color goes from 0 to 14 -- background color stays 0 (black)
    if (fc > 14) { // it goes up to 14 (0xE) = white
      fc = 2;   
    }

    vga.setTextColor(fc, 0);                 // keep background on 0 (black)
    vga.println(lineNumber++, HEX);          // println() will force screen scroll up!

    lastLineNumber = ++lastLineNumber & 0xF; // increments and keeps the range in 0..15 for a single color index in the sequence we defined
    // prints a number (color code) with leading '0's and 2 digits, after scrolling....
    vga.printInt (24, 29, lastLineNumber, vga.getColorCode(RGB_BLACK, RGB_WHITE), true, 2); // helper function to print integers

    if (!lineNumber) {                       // on byte overflow, reset the screen
      ScreenSetup();
      vga.setTextCursor(0, 0);               // set cursor to default top left corner postiion
    }
  }
}

void ScreenSetup (void) {

  vga.clearScreen();


//  testing exmaple for scrolling the text with println()
//  for(uint32_t i=0;;i++) {                      // prints increasing numbers for ever and scrolls the screen when reaching last line
//    vga.print("value of i is ");
//    vga.println(i);
//    vga.waitVSync();                            // waits to end of VGA frame drawing routine to end -- makes the image to stay solid
//  }

  
  vga.printStr(0, 20, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"1234567890123456789012345678");

  // draws charcater #127 (checker) in 3 rows in the screen, alternating colors, keeping Red as main color
  for (uint8_t i = 0; i < VRAM_WIDTH; i++) {  
    // there are 3 ways for drawing a single character in the screen, using setTile(...):
    vga.setTile(i, 22, 127, vga.getColorCode(RGB_RED, RGB_YELLOW));   // using setTile(...) with a color at last paramenter
    vga.setTile(i, 23, 127, RGB_BLACK, RGB_RED);                      // using setTile(...) with arguments separated for foreground and background colors
    vga.setTile(i, 24, 127);                                          // using setTile(...) with no color argument, thus it doesn't change the colors of that x,y position
    vga.setColor(i, 24, vga.getColorCode(RGB_RED, RGB_WHITE));        // then it may be possible to only change the color at x,y not changing the tile at that position
  }

#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
  vga.printStr(5, 0, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"Roger's Core DEMO!");
#endif

#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
  vga.printStr(6, 0, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"STM32 Core DEMO!");
#endif

  // prints all possible colors with text in the screen
  vga.printStr(7, 11, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)    "-----RED------");
  vga.printStr(7, 12, vga.getColorCode(RGB_MAGENTA, RGB_BLACK), (char *)"---MAGENTA----");
  vga.printStr(7, 13, vga.getColorCode(RGB_BLUE, RGB_BLACK), (char *)   "-----BLUE-----");
  vga.printStr(7, 14, vga.getColorCode(RGB_CYAN, RGB_BLACK), (char *)   "-----CYAN-----");
  vga.printStr(7, 15, vga.getColorCode(RGB_GREEN, RGB_BLACK), (char *)  "----GREEN-----");
  vga.printStr(7, 16, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *) "----YELLOW----");
  vga.printStr(7, 17, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)  "----WHITE-----");
  vga.printStr(7, 18, vga.getColorCode(RGB_BLACK, RGB_WHITE), (char *)  "BLACK REVERSED");
  vga.printStr(7, 29, vga.getColorCode(1, RGB_BLACK), (char *)  "--Last Line--");

  // prints the basic ASCII table in the screen
  for (uint8_t y = 3; y < 9; y++)
    for (uint8_t x = 6; x < 22; x++) {
      vga.setTile(x, y, 26 + x + (y - 3) * 16);
      vga.setColor(x, y, vga.getColorCode(RGB_WHITE, RGB_BLACK));
    }
}


