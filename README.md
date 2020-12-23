![BlueVGA View](https://rogabeweb.mybluemix.net/images/BlueVGA.png)

Demo video here: [http://www.youtube.com/watch?v=nNWkdR4P_UA](http://www.youtube.com/watch?v=nNWkdR4P_UA)

This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/).

# BlueVGA v1.2
VGA library for STM32F103C8T6 (BluePill) that can manipulate a 224x240 pixels screen with 8x8 Tiles (or Characters) from a user defined bitmap font

In version 1.2 there is a graphical mode that allows to manipulate pixels as well. Actually it uses 256 RAM Tiles that can be positioned any place in the screen and, by this way, it is possible to set individual pixels.

This is not a full screen pixel setting, but allows for several graphical applications. Please check the examples in this library to learn more about it.

This graphical mode can be used to create some animation such as in the Space_Invaders_Animation_Demo sketch found in examples - screenshot:


![Space Invaders Screen Shot](https://rogabeweb.mybluemix.net/images/SpaceInvaders.png)

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

## What can be done with this Library?
BlueVGA is an Arduino compatible VGA library that can be used to display text and graphics to a VGA monitor.
It uses a very low footprint (RAM & Flash).
BlueVGA uses only about 1.5KB RAM and 8K Flash, leaving a lot of room for your sketch and data.

This library is compatible with Print Class, therefore it is possible to use print() and println() functions from BlueVGA class.
This works in the same way as Serial.print() and Serial.println() but its output is displayed on the VGA screen, with text scrolling.
In the examples there is the printTest.ino that demonstrates this capabilities.

## Functions related to text printing:
```javascript
#include "bluevga.h"
#include "font.h"

BlueVGA vga(ASCII_FONT);   // starts VGA driver using bitmap array ASCII_FONT

// It is based on 'hello_world.ino' that can be found in the library examples 
void setup() {
  vga.setTextCursor(0, 1);                 // position cursor at x = 0 and y = 1 (second line) in the screen
  vga.print("Hello World\t\t");            // it supports all Arduino way for println() and understands '\t' as tab, default 4 columns
  vga.print("tab space\n");                // it also understands '\n' as new line and '\r' as carriage return in the same line
  vga.setTextWrap(false);                  // it allows to wrap text or not when printing beyond right margin of the screen
  vga.setTextTab(3);                       // it allows to set number of columns for TAB ('\t')
  vga.setTextColor(RGB_GREEN, RGB_BLACK);  // it allows to define foreground (green) and background (black) colors for further printing
  vga.print(3.1415, 3);                    // it works as Arduino, printing floats witn any number of decimal digits 
  uint8_t color = vga.getTextColor();      // gets current colors in format 8 bits rgb0rgb0 BGColor 4 bits followed by FGColor 4 bits
  uint8_t x = vga.getTextCursorX();        // gets current X cursor position
  uint8_t y = vga.getTextCursorY();        // gets current Y cursor position
  vga.scrollText(3);                       // it scrolls the screen up 3 lines, no change on current cursor position. Just scrolls text and its colors!
  for(uint32_t i=0;;i++) {                 // prints increasing numbers for ever and scrolls the screen when reaching last line
    vga.print("value of i is ");
    vga.println(i);
    vga.waitVSync();                       // waits to end of VGA frame drawing routine to end -- makes the image to stay solid
  }
}


```

## Functions related to graphical drawing:

In this version, it's possible to set individual pixels using the new class **_BlueBitmap_**.

It actually uses RAM Tiles instead of Flash Tile, which in turn allow to set an individual pixel within this tile.
There are 256 RAM Tiles, thus, in a 16 x 16 tile arrangement, we could create a full canvas of 128 x 128 pixels.
But those RAM Tiles can be any place in the 28x30 tile space.
This allows to create, for instance, an animation such as Space Invaders (in the examples), or even create widgets for the user interface.

#### Main Methods are:
```javascript
void BlueBitmap::drawPixel(uint8_t x, uint8_t y, bool setReset) // to draw a pixel on any place of the screen
void BlueBitmap::drawBitmap(uint8_t x, uint8_t y, uint8_t frameNum, bool setReset, int8_t color) // to draw a bitmap on the screen
void BlueBitmap::drawBitmap8(uint8_t x, uint8_t y, uint8_t frameNum, bool setReset, int8_t color) // to fast draw of 8x8 or 16x8 bitmaps

void setBlueVgaObject(BlueVGA &bvgaPtr) // to associate the vga object to drawing functions 

// RAM Tiles can be split into two sections: RAM Tiles that are "safe" and won't be used in a drawPixel() 
// and the rest that will be dynamically allocated to display pixels in the screen. By default all the tiles 
// are available for drwaing pixels.
void BlueBitmap::eraseRamTiles() 	// to reset the graphical environment - uses firstTile information
void setFirstTile(uint8_t tile)		// to assign the Tile number that a graphical function can use to set a pixel 
 					// when all available tiles are used it will recycle back to this tile
void setNextFreeTile(uint8_t tile)  	// to assign the Tile number that a graphical function can use to set a pixel
 					// used as next available tile for pixels
```

#### New Examples in Github:
* **Space Invaders Animation Demo** - a nice auto-play Space Invaders Animation
* **Elliptical Text Animation** - an animation of text moving in different elliptical paths
* **graph Demo Sine Curve** - an slow motion animation of a sine curve drawing. At the end it changes to character view in order to help understanding how the graphical mode works.
Top

### Some Examples

Click the image below to watch it in action (you will be redirected to YouTube)

[![Video Snake Game](http://img.youtube.com/vi/nNWkdR4P_UA/0.jpg)](http://www.youtube.com/watch?v=nNWkdR4P_UA)

![Image2](https://rogabeweb.mybluemix.net/images/example2.png)

## Connections

BlueVGA uses pins: 
  * **PA9** as Horizontal Sync VGA signal
  * **PB6** as Vertical Sync VGA signal
  * **PC13** for Blue VGA signal
  * **PC14** for Green VGA signal
  * **PC15** for Red VGA signal
  * **G** (BluePill ground) must be connected to GND pin of VGA

![VGA Connection](https://rogabeweb.mybluemix.net/images/BlueVGA_Connection.png)

VGA signal goes from 0 volt to 0.7 volt. Bluepill works with 0 to 3.3 volts.
In order to level the output to a voltage close to 0.7, it's necessary to add a 330R resistor between STM32F103 port and VGA connected pin.
VGA has a 75R grounded resistor to RGB pins, thus 330R will work as voltage divider and take 3.3 volts to something about 0.6 volts.

But most modern VGA monitors support 3.3 volts directly, therefore STM32F103 pins could be connected directly VGA using wires or jumpers, as in the images:

![Jumpers on VGA](https://rogabeweb.mybluemix.net/images/VGA_Jumpers.png)

![Jumpers](https://rogabeweb.mybluemix.net/images/Bluepill_Joystick.png)

## BlueVGA _Screen_
BlueVGA _Screen_ is composed of 28x30 tiles. 
Each tile has only 2 possible colors, a foreground and background color. 
BlueVGA is designed to display 8 different colors: black, blue, cyan, green, yellow, red, magenta and white.

![BlueVGA Image 8 Colors](https://rogabeweb.mybluemix.net/images/8colors.png)

### What are Tiles?
A tile can be as simple as a character.
A tile is a bitmap of 8x8 pixels (bits). Bit 1 is foreground and Bit 0 background.
This bitmap can represent a letter or any graphic drawing with 8x8 pixels.

![IMAGE Char A](https://rogabeweb.mybluemix.net/images/charA.png)

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

![Hello World](https://rogabeweb.mybluemix.net/images/HelloWorld.jpg)


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
This is done by adding a file named "build_opt.h", that does nothing and is not compiled, but tells STM32 Core to use its content as compiling options.
Beside that effect there is an important side effect: **Any Arduino function that relies on a HardwareTimer won't work**.
Some known fuctions that need Timer are: HardwareTimer functions, Servo Motor control functions, tone() related functions, pwmWrite() and pinMode(..., PWM) functions
In order to use any of those functions, unfortunately, user will have to write a new version of necessary functions or use some 3rd party library to replace them.

For work aroud, it's possible to use Roger's Core instead, which doesn't turn off HardwareTimer.
