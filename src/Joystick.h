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

#ifndef Joystick_h
#define Joystick_h

/*
 * 
 * This Class can be used with an Arduino Analog Joystick like the one viewed in this video  https://www.youtube.com/watch?v=f8iaW5YcsNE 
 * or it can also handle Push Buttons for each direction and another for "fire".
 * When creating the instance of this class, if pin used for left and right are the same, it assumes it is been used an analog joytick for left/right
 * The same for up and down.
 * 
 * Usage Examples:
 * Joystick joystick(PA3, PA3, PA4, PA4, PB5);  // assumes an analog joystick using PA3 for right/left, PA4 for up/down and PB5 for "fire"
 * Joystick joystick(PA1, PA2, PA3, PA4, PB5);  // assumes ues of push buttons which are in PULL_UP state, thus by driving each pin to Ground will activate it
 * Joystick joystick(PA3, PA3, PA4, PA5, PB5);  // assumes a mixing of analog joystick with push butom, by using PA3 for right/left (analog), and digital signals in PA4 up, PA5 down and PB5 "fire"
 *
 */

#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
// Analog reads from 0 to 4096 (12 bits)
#define TOP_VALUE      4000
#define BOTTOM_VALUE   400
#endif

#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
// Analog reads from 0 to 1024 (10 bits)
#define TOP_VALUE      900
#define BOTTOM_VALUE   100
#endif



class Joystick {
 private:
  uint8_t leftPin;
  uint8_t rightPin;
  uint8_t upPin;
  uint8_t downPin;
  uint8_t firePin;

 public:
    void initController(uint8_t left, uint8_t right, uint8_t up, uint8_t down, uint8_t fire);
    Joystick();
    void configController(uint8_t left, uint8_t right, uint8_t up, uint8_t down, uint8_t fire);
    Joystick(uint8_t left, uint8_t right, uint8_t up, uint8_t down, uint8_t fire);
  
    uint8_t leftPressed();
    uint8_t rightPressed();
    uint8_t upPressed();
    uint8_t downPressed();
    uint8_t firePressed();
    uint8_t anyPressed();
};

#endif


