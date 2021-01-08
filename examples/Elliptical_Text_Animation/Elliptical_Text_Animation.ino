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
#include "bluebitmap.h"      // functions for drawing pixels in the screen using 256 RAM Tiles

// USE_RAM means that we shall use an empty RAM space for 256 tiles of 8x8 pixels
// creates a BlueVGA object using RAM Tiles as bitmap for graphical usage
BlueVGA vga(USE_RAM);

// creates an object that maps bitmap to an ASCII font with 8x8 pixels
BlueBitmap fontBitmap(8, 8, (uint8_t *)ASCII_FONT);


void setup() {
  // clears the screen using blue background color with yellow as foreground color
  // FG Color = Yellow || BG Color = Blue using 3 bits per color in format RGB0 (4 bits with a bit 0 = 0)
  BlueBitmap::clearGraphScreen(vga.getColorCode(RGB_YELLOW, RGB_BLUE));
}

void loop() {
  const char text[] = "BlueVGA";
  const uint8_t textLen = strlen(text);

  // Vertical Radius proportion to the Horizontal Radius and a change its factor along time
  static float ellipseProp = -0.5, propDir = 0.2;   
  // Ellipse coordinates description
  float h = 90.0;     // horizontal center 
  float k = 125.0;    // vertical center
  float r = 50.0;     // radius of a circunference. ellipseProp will change vertical radius to create a ellipse

  // goes around 360 degrees in radians (360 = 2PI) stepping by 5 degrees 
  for (float angule = 0; angule < TWO_PI; angule += (TWO_PI / 360 * 5)) { // 5 degrees change -- speed of the animation
    // calculates the coordinates of the text based on an ellipse curve that changes its proportions on each loop() execution
    float textX = h + r * cos(angule);
    float textY = k + ellipseProp * r * sin(angule);

    // grafically draws the text in the screen based on the ellipse coordinates
    for (uint8_t i = 0; i < textLen; i++) {
      //BlueBitmap::drawPixel(textX, textY, true);  // alternative way to see the elliptical movement
      fontBitmap.drawBitmap8((uint8_t) textX + i * 8, (uint8_t) textY, text[i], 1, vga.getColorCode(RGB_BLUE, RGB_BLUE));
    }
    
    // the animation is excecuted 30 times per second... causing some optical ilusions on movement
    vga.waitVSync(2); // blocks the execution until 2 frames are past. At 60 FPS (frames per second) 2/60s = 33 milliseconds

    // clears the screen using blue background color with yellow as foreground color
    BlueBitmap::clearGraphScreen(vga.getColorCode(RGB_YELLOW, RGB_BLUE));
  }

  // cyclically changes the shape (proportions) of the ellipse
  ellipseProp += propDir;
  if (ellipseProp > 2 and propDir > 0) {
    propDir = -propDir;
    ellipseProp = 2;
  }
  if (ellipseProp < -0.5 and propDir < 0) {
    propDir = -propDir;
    ellipseProp = -0.5;
  }
}