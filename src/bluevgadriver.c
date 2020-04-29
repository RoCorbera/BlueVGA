/*
   BlueVGA library - VGA Driver Library for STM32F103

   - This library is intended to work in Arduino IDE using Bluepill STM32F103C8 or STM32F103CB boards
   - It uses ARDUINO Roger's core for STM32F103C board. Please check it at https://github.com/rogerclarkmelbourne/Arduino_STM32
     you will find arduino installation for Roger's core at https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/Installation
   - It was tested and runs using the following Arduino Settings for the board:
       Generic STM32F103C series
       Optimize Os (Smallest)
       Variant STM32F103C8 or STM32F103CB
       CPU Speed(MHz) 72MHz (Normal)

    Author Rodrigo Patricio Garcia Corbera (rocorbera@gmail.com)
    Copyright © 2017-2020 Rodrigo Patricio Garcia Corbera. 
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

#include <stdint.h>
#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <libmaple/rcc.h>

#include "bluevgadriver.h"

volatile uint32_t frameNumber = 0;

uint8_t TRAM [VRAM_HEIGHT][VRAM_WIDTH] __attribute__((aligned(32))); // VRAM with static Tiles of 8 x 8 pixels
uint8_t CRAM [VRAM_HEIGHT][VRAM_WIDTH] __attribute__((aligned(32))); // Color VRAM - 8 + 8 colors = 4bits + 4bits (Foreground + Background)
uint8_t *TBitmap;

void scanLine(uint8_t *Tiles, uint8_t *Colors, const uint8_t *Bitmap, const uint8_t *gpio, uint8_t *Buffer) {

  // assembly for sending the scanline to VGA Monitor using the 3 most significant bits for Red, Green and Blue
  __asm__ volatile (
    "  mov r6, r0                  \n\t"
    "  mov r7, r2                  \n\t"
    "  mov r8, r4                  \n\t"
    ".rept 28                      \n\t"
    "  ldrb r5, [r6], #1           \n\t"
    "  ldrb r9, [r7, r5, LSL#3]    \n\t"
    "  strb r9, [r8], #1           \n\t"
    ".endr                         \n\t"
    "  mov r8, r4                  \n\t"
    "  mov r7, r1                  \n\t"
    "  ldr r5, [r8], #4            \n\t"
    "  ldr r9, [r7], #4            \n\t"
    ".rept 7                       \n\t"
    "  ror r5, r5, #5              \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".rept 7                       \n\t"
    "  nop                         \n\t"
    "  ror r5, r5, #31             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".endr                         \n\t"
    "  ror r9, r9, #8              \n\t"
    "  ror r5, r5, #15             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".rept 7                       \n\t"
    "  nop                         \n\t"
    "  ror r5, r5, #31             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".endr                         \n\t"
    "  ror r5, r5, #15             \n\t"
    "  ror r9, r9, #8              \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  strb r6, [r3]               \n\t"
    ".rept 7                       \n\t"
    "  nop                         \n\t"
    "  ror r5, r5, #31             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".endr                         \n\t"
    "  ror r9, r9, #8              \n\t"
    "  ror r5, r5, #15             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".rept 3                       \n\t"
    "  nop                         \n\t"
    "  ror r5, r5, #31             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    ".endr                         \n\t"
    "  ror r5, r5, #31             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  ror r5, r5, #31             \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    "  ldr r0, [r8], #4            \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    "  ror r5, r5, #31             \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  ror r5, r5, #31             \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    "  and r6, r5, #4              \n\t"
    "  lsl r6, r9, r6              \n\t"
    "  ldr r9, [r7], #4            \n\t"
    "  nop                         \n\t"
    "  strb r6, [r3]               \n\t"
    "  mov r5, r0                  \n\t"
    ".endr                         \n\t"
    "  nop                         \n\t"
    "  mov r5, #0                  \n\t"
    "  nop                         \n\t"
    "  nop                         \n\t"
    "  nop                         \n\t"
    "  nop                         \n\t"
    "  strb r5, [r3]               \n\t"
    :
    : "r" (Tiles), "r" (Colors), "r" (Bitmap), "r" (gpio), "r" (Buffer)
    : "r5", "r6", "r7", "r8", "r9"
  );
}

// This is the main horizontal sweep - designed to display 224x240 pixels, 8 colors (3bpp) on a VGA 640x480@60Hz signal, Red (PC15) Green(PC14) Blue(PC13)
void __attribute__((optimize("O3"))) sendScanLine(void) {

  static uint8_t linePixel = 0;
  static uint8_t videoOn = 0;
  static uint32_t scanLineCounter = 0;
  static uint8_t bitmap[VRAM_WIDTH] __attribute__((aligned(32)));
  const uint8_t *GPIO __attribute__((aligned(32))) = (uint8_t*)(&(GPIOC_BASE)->ODR);

  if (videoOn) scanLine(TRAM[linePixel >> 3], CRAM[linePixel >> 3], TBitmap + (linePixel & 7), GPIO, bitmap);

  scanLineCounter++; // scanLineCounter increments every 1/(525*60) = 31.75 us @ 31.5KHz
  if (!(scanLineCounter & 1)) {
    linePixel++;
  }
  if (TIMER4_BASE->CNT == 515) {
    videoOn = 0;
    frameNumber++;
  }
  if (TIMER4_BASE->CNT == 35) {
    videoOn = 1;
    linePixel = 0;
  }
}


// starts timers for generating VGA sinal from pins A9 (VGA HSync) and B6 (VGA Vsync) for 640x480@60Hz
void video_init() {

  RCC_BASE->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN;
  RCC_BASE->APB1ENR |= RCC_APB1ENR_TIM4EN;
  RCC_BASE->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
  GPIOA_BASE->CRH = (GPIOA_BASE->CRH & 0xFFFFFF0F) | 0x000000B0;
  GPIOB_BASE->CRL = (GPIOB_BASE->CRL & 0xF0FFFFFF) | 0x0B000000;
  GPIOC_BASE->CRH = 0x33333333;

  TIMER1_BASE->CCER |= 0x10;
  TIMER1_BASE->CR1 &= ~0x1;
  TIMER1_BASE->CR2 = 0x20;
  TIMER1_BASE->PSC = 0;
  TIMER1_BASE->CNT = 0;
  TIMER1_BASE->ARR = 2287;
  TIMER1_BASE->CCR2 = 142;
  TIMER1_BASE->CCR1 = 10;
  TIMER1_BASE->CR1 |= 0x80;
  TIMER1_BASE->CCMR1 |= 0x7800;

  TIMER4_BASE->CCER |= 0x1;
  TIMER4_BASE->CR1 &= 0xFFFE;
  TIMER4_BASE->PSC = 0;
  TIMER4_BASE->ARR = 525;
  TIMER4_BASE->CNT = 0;
  TIMER4_BASE->CCR1 = 1;
  TIMER4_BASE->CR1 |= 0x80;
  TIMER4_BASE->CCMR1 |= 0x0078;
  TIMER4_BASE->SMCR = (TIMER4_BASE->SMCR & 0x88) | 0x07;

  timer_attach_interrupt(&timer1, TIMER_CH1, sendScanLine);

  TIMER4_BASE->CR1 |= 0x1;
  TIMER1_BASE->CR1 |= 0x1;
}

// stops timers and VGA sinal
void video_end() {
  TIMER1_BASE->CR1 &= ~0x1;
  TIMER4_BASE->CR1 *= ~0x1;
}

