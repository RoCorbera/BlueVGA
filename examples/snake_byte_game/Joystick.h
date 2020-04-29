/*
   BlueVGA library - VGA Driver Library for STM32F103

   - This library is intended to work in Arduino IDE using Bluepill STM32F103C8 or STM32F103CB boards
   - It uses ARDUINO Roger's core for STM32duino board. Please check it at https://github.com/rogerclarkmelbourne/Arduino_STM32
     you will find arduino installation for Roger's core at https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/Installation

   - It was tested and runs using the following Arduino Settings for the board:
       Generic STM32F103C series
       Optimize Os (Smallest) or O1 (Fast)
       Variant STM32F103C8 or STM32F103CB
       CPU Speed(MHz) 72MHz (Normal)

    Created by Rodrigo Corbera (rocorbera@gmail.com)

    This code is licensed as Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0) - https://creativecommons.org/licenses/by-nc-sa/4.0/
    Full legal language of the license here granted can be found at https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

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


