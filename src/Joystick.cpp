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

#include <Arduino.h>
#include "Joystick.h"


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
 *  MIT license
 *
 */


Joystick::Joystick() {
  initController(PA3, PA3, PA4, PA4, PB5);
}

Joystick::Joystick(uint8_t left, uint8_t right, uint8_t up, uint8_t down, uint8_t fire) {
  initController(left, right, up, down, fire);
}

void Joystick::configController(uint8_t left, uint8_t right, uint8_t up, uint8_t down, uint8_t fire) {
  initController(left, right, up, down, fire);
}

void Joystick::initController(uint8_t left, uint8_t right, uint8_t up, uint8_t down, uint8_t fire) {
  this->leftPin = left;
  this->rightPin = right;
  this->upPin = up;
  this->downPin = down;
  this->firePin = fire;

  if (leftPin == rightPin) {
    pinMode(leftPin, INPUT_ANALOG);
  } else  {
    pinMode(leftPin, INPUT_PULLUP);
    pinMode(rightPin, INPUT_PULLUP);

  }
  if (upPin == downPin) {
    pinMode(upPin, INPUT_ANALOG);

  } else  {
    pinMode(upPin, INPUT_PULLUP);
    pinMode(downPin, INPUT_PULLUP);

  }
  pinMode(firePin, INPUT_PULLUP);
}


uint8_t Joystick::leftPressed(void) {

  if (leftPin == rightPin) {
    if (analogRead(leftPin) > TOP_VALUE) return 1;
    else return 0;

  } else {
    if (digitalRead(leftPin) == LOW) {
      return 1;
    } else {
      return 0;
    }
  }

}

uint8_t Joystick::rightPressed(void) {
  if (leftPin == rightPin) {
    if (analogRead(leftPin) < BOTTOM_VALUE) return 1;
    else return 0;

  } else {
    if (digitalRead(rightPin) == LOW) {
      return 1;
    } else {
      return 0;
    }
  }

}

uint8_t Joystick::upPressed(void) {
  if (upPin == downPin) {
    if (analogRead(upPin) > TOP_VALUE) return 1;
    else return 0;

  } else {
    if (digitalRead(upPin) == LOW) {
      return 1;
    } else {
      return 0;
    }
  }

}

uint8_t Joystick::downPressed(void) {
  if (upPin == downPin) {
    if (analogRead(upPin) < BOTTOM_VALUE) return 1;
    else return 0;

  } else {
    if (digitalRead(downPin) == LOW) {
      return 1;
    } else {
      return 0;
    }
  }

}

uint8_t Joystick::firePressed(void) {
  if (digitalRead(firePin) == LOW) {
    return 1;
  } else {
    return 0;
  }
}



uint8_t Joystick::anyPressed (void) {
  if (this->firePressed() || this->upPressed() || this->downPressed() || this->leftPressed()  || this->rightPressed()) return 1;
  return 0;
}





