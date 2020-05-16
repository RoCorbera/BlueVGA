![BlueVGA View](https://rogabeart.mybluemix.net/images/BlueVGA.png)

This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/).

# BlueVGA v1.0
VGA library for STM32F103C8T6 (BluePill) that can manipulate a 224x240 pixels screen with 8x8 Tiles (or Characters) from a user defined bitmap font

## How to use it with Arduino IDE
This library was designed to work with Arduino IDE using the STM32F103C8, aka [Bluepill](https://stm32duinoforum.com/forum/wiki_subdomain/index_title_Blue_Pill.html)
It olny runs with Arduino board configuration called **Generic STM32F1 Series** or **Generic STM32F103C Series**, depending on the choosen core.

### This library is compatible with the 2 most known "Arduino Cores", in a **transparently way**:

**Option 1**: The STMicroelectronics Official Core known as STM32 Core. This library only runs on minimum version 1.9.0 of STM32 Core

In order to install this board to Arduino IDE, please refer to [this guide](https://github.com/stm32duino/wiki/wiki/Getting-Started)

For more information about STM32 Core [click here](https://github.com/stm32duino/Arduino_Core_STM32)
For doubts or questions about how to use it, please go to this [forum](https://www.stm32duino.com/)


**Option 2**: The community Core known as Roger's Core.

In order to install this board to Arduino IDE, please refer to [this guide](https://stm32duinoforum.com/forum/wiki_subdomain/index_title_Installation.html)

For more information about Roger's Core [click here](https://github.com/rogerclarkmelbourne/Arduino_STM32)
For doubts or questions about how to use it, please go to this [forum](https://www.stm32duino.com/)

## What can I do with this Library?
BlueVGA is an Arduino compatible VGA library that can be used to display almost anything to a VGA monitor.
It uses a low footprint (RAM & Flash) in order to be light enough to leave space for your sketch.

### Some Examples

![Image1](https://rogabeart.mybluemix.net/images/example1.png)

![Image2](https://rogabeart.mybluemix.net/images/example2.png)

## Connections

BlueVGA uses pins: 
  * **PA9** as Horizontal Sync VGA signal
  * **PB6** as Vertical Sync VGA signal
  * **PC13** for Blue VGA signal
  * **PC14** for Green VGA signal
  * **PC15** for Red VGA signal

VGA signals go from 0 volt to 0.7 volt. Bluepill works with 0 to 3.3 volts.
In order to level the output to a voltage close to 0.7, it's necessary to add a 330R resistor between STM32F103 port and VGA connected pin.
VGA has a 75R grounded resitor in RGB pins, thus 330R will work as voltage divider and take 3.3 volts to something about 0.6 volts.

But most moderns VGA monitors support 3.3 volts directly, therefore STM32F103 pins could be connected directly VGA using wires or jumpers, as in the images:

![Jumpers on VGA](https://rogabeart.mybluemix.net/images/VGA_Jumpers.png)

![Jumpers](https://rogabeart.mybluemix.net/images/Bluepill_Joystick.png)

## BlueVGA _Screen_
BlueVGA _Screen_ is composed of 28x30 tiles. 
Each tile has only 2 possible colors, a foreground and background color. 
BlueVGA is designed to display 8 different colors: black, blue, cyan, green, yellow, red, magenta and white.

![BlueVGA Image 8 Colors](https://rogabeart.mybluemix.net/images/8colors.png)

### What are Tiles?
A tile can be as simple as a character.
A tile is a bitmap of 8x8 pixels (bits). Bit 1 is foreground and Bit 0 background.
This bitmap can represent a letter or any graphic drawing with 8x8 pixels.

![IMAGE Char A](https://rogabeart.mybluemix.net/images/charA.png)

A tile has 8 bytes (64 bits).
For representing the character 'A' as in the image above, we can create an array such as:

```javascript

// Font array for letter A bitmap
// each tile has olny two colors out of 8 possible colors
// bit 0 is background color 
// bit 1 is foreground color
//
// It can be represented as binary number for making it easy to view the drawing
// but also can use hexa code such as 0x24 = 36 = 0B00100100
// It has 8x8 bits = 8 bytes to define 'A' letter

const uint8_t bitmap_A = {
   0B00000000,      // ________ 
   0B00011000,      // ___XX___
   0B00100100,      // __X__X__
   0B00100100,      // __X__X__
   0B00111100,      // __XXXX__
   0B00100100,      // __X__X__
   0B00100100,      // __X__X__
   0B00000000       // _________
};

```
## Coding!

### Sketch example

```javascript
#include "bluevga.h"
#include "font.h"

BlueVGA vga(ASCII_FONT);   // starts VGA driver using bitmap array ASCII_FONT

// It is based on 'hello_world.ino' that can be found in the library examples 
void setup() {
}

// Blink the text 'Hello World!"
void loop() {
  vga.clearScreen();  // clears the screen to black
  vga.waitVSync(60);  // same as delay(1000); -- it delays by 60 frames in a 60 frames per second system
  
  // prints the text at position 1,2 using yellow as foreground color on a black background
  vga.printStr(1, 2, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"Hello World!"); 
  vga.waitVSync(60);  // same as delay(1000); 
}

```

Result of this sketch:
![Hello World](https://rogabeart.mybluemix.net/images/HelloWorld.jpg)


### Documentation and functions

Please look at "BlueVGA.h" for further information on each possible function of the library
There are also good examples in the library 

# Important Information

This library **halts SysTime functionality** in order to generate a solid and clear image on the screen.
Thus functions such as delay(), millis(), micros(), delayMicroseconds() **will not work**!
Instead of those functions, BlueVGA provides two alternatives:

```javascript

// each Frame is displayed 60 times per second.
// waitVSync() holds the executuion for 1/60 of a second or 16.66 milliseconds
void waitVSync(uint16_t waitFrames = 1);

// returns the number of frames already displayed since VGA driver started
// it also represents time beacuse each frame is drawn at each 16.66 milliseconds
// thus it can be used instead of millis() using as unit 1/60 of a second instead of 1/1000 of a second
int32_t getFrameNumber();          

```

**Another important information** is related to use of STM32 Core. In order to achieve a nice VGA resoltution, **BlueVGA turns off HardwareTimer** in this core.
This is done bu adding a file named "build_opt.h", that does nothing and is not compiled, but tells STM32 Core to use its content as compilation options.
Beside that effect there is an important side effect: Any Arduino function that relies on a Timer won't work.
Some known fuctions that need Timer are: HardwareTimer functions, Servo Motor control functions, tone() related functions, pwmWrite() and pinMode(..., PWM) functions
In order to used any of those functions, unfortunately, user will have to write a new version of it or use some 3rd party library to replace them.

