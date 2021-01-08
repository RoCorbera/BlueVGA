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
#include "font.h"            // imports a ASCII Flash font bitmap
#include "bluebitmap.h"      // functions for drawing pixels in the screen

// USE_RAM means that we shall use an empty RAM space for 256 tiles of 8x8 pixels
// creates a BlueVGA object using RAM Tiles as bitmap for graphical usage
BlueVGA vga(USE_RAM);

// Some helpfull constants for radians calculations
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

// defines number of pixels of the whole screen
#define SCR_WIDTH  VRAM_WIDTH * 8   // 28 * 8 = 224 pixels
#define SCR_HEIGHT VRAM_HEIGHT * 8  // 30 * 8 = 240 pixels

// defines a pixel sine curve area of 200 x 200 pixels
#define CANVASW     200
#define CANVASH     200
// pixel sine curve left top corner coordinates
#define XCORNER     ((SCR_WIDTH - CANVASW)/2)
#define YCORNER     ((SCR_HEIGHT - CANVASH)/2)
// pixel sine curve center coordinates
#define XCENTER     (XCORNER + CANVASW/2)
#define YCENTER     (YCORNER + CANVASH/2)

const char title[] = "SINE CURVE GRAPHICS";

void setup() {
  // fills up the screen with tileID zero using yellow as foreground color over black as background color
  BlueBitmap::clearGraphScreen(vga.getColorCode(RGB_YELLOW, RGB_BLACK));

  // creates an object that maps bitmap to a ASCII font 8x8
  BlueBitmap fontBitmap(8, 8, (uint8_t *)ASCII_FONT);

  // draws pixels of the title in cyan
  uint8_t titleLen = strlen(title);
  for (uint8_t i = 0; i < titleLen; i++)
    fontBitmap.drawBitmap8((VRAM_WIDTH - titleLen) / 2 * 8 + i * 8, 0, title[i], 1, vga.getColorCode(RGB_CYAN, RGB_BLACK));

  // Draws two lines, a left y axis and a centered x axis
  for (uint8_t x = 0; x < SCR_WIDTH; x++) BlueBitmap::drawPixel(x, YCENTER);
  for (uint8_t y = YCORNER; y < YCORNER + CANVASH; y++) BlueBitmap::drawPixel(XCORNER, y);

  // works as a delay(2000);
  // blocks the execution until 120 frames are past.
  // At 60 FPS (frames per second) 120/60 = 2 seconds
  vga.waitVSync(120);
}

void loop() {

  // TWO_PI is 360 degrees in radians - iterate at CANVASW * 4 times over a sine curve
  for (float angule = 0; angule < TWO_PI; angule += (TWO_PI / CANVASW / 4)) {

    // this delay creates a drawing animation
    vga.waitVSync(2); // delays loop by 1/30 sencond

    // calculates angle proportional to Canvas width in pixels
    // and transladate it to X axis origin
    int16_t xpos = angule / TWO_PI * CANVASW + XCORNER;

    // sin() goes from 1 to -1 thus we have just to make it proportional
    // and transladate it to the center of the graphic
    int16_t ypos = sin(angule) * CANVASH / 2 + YCENTER;

    // draws pixel by pixel
    BlueBitmap::drawPixel(xpos, ypos);
  }

  vga.waitVSync(120); // same as delay(2000); - hold it for 2 seconds

  // change the font of tile to a flash ASCII_FONT, just to demonstrate how tile are
  // allocated when drawing pixels. Just for demonstration effect.
  vga.setFontBitmap((uint8_t *)font8x8_ic8x8u + 32 * 8);  // right after ' ' (ASCII 0x20 = 32)

  // vga.waitVSynch() blocks the execution until a whole VGA frame has been fully drawn
  // this also make the VGA image to be clear and solid
  while (true) vga.waitVSync(); // Halts the execution for ever...
}



