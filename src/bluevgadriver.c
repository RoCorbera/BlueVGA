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

#include <stdint.h>

#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <libmaple/rcc.h>

#define RCC_REG    RCC_BASE
#define GPIOA_REG  GPIOA_BASE
#define GPIOB_REG  GPIOB_BASE
#define GPIOC_REG  GPIOC_BASE
#define TIM1_REG   TIMER1_BASE
#define TIM4_REG   TIMER4_BASE
#endif

#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
#include <stm32f103xb.h>
#include <stm32f1xx_hal.h>

#define RCC_REG    RCC
#define GPIOA_REG  GPIOA
#define GPIOB_REG  GPIOB
#define GPIOC_REG  GPIOC
#define TIM1_REG   TIM1
#define TIM4_REG   TIM4
#endif

#include "bluevgadriver.h"

volatile uint32_t frameNumber = 0;
volatile uint32_t scanLineCounter = 0;

uint8_t TRAM [VRAM_HEIGHT][VRAM_WIDTH] __attribute__((aligned(32))); // VRAM with static Tiles of 8 x 8 pixels
uint8_t CRAM [VRAM_HEIGHT][VRAM_WIDTH] __attribute__((aligned(32))); // Color VRAM - 8 + 8 colors = 4bits + 4bits (Foreground + Background)

uint8_t *TBitmap;

void sendScanLine(void) __attribute__((aligned(32)));
void scanLine(uint8_t *Tiles, uint8_t *Colors, const uint8_t *Bitmap, const uint8_t *gpio, uint8_t *Buffer) __attribute__((aligned(32)));


// function size 4480
void scanLine(uint8_t *Tiles, uint8_t *Colors, const uint8_t *Bitmap, const uint8_t *gpio, uint8_t *Buffer) {
  // assembly for sending the scanline to VGA Monitor using the 3 most significant bits for Red, Green and Blue
  asm volatile (
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
  static uint8_t bitmap[VRAM_WIDTH] __attribute__((aligned(32)));
  const uint8_t *GPIO __attribute__((aligned(32))) = (uint8_t*)(&(GPIOC_REG)->ODR);

  if (videoOn) scanLine(TRAM[linePixel >> 3], CRAM[linePixel >> 3], TBitmap + (linePixel & 7), GPIO, bitmap);

  scanLineCounter++; // scanLineCounter increments every 1/(525*60) = 31.75 us @ 31.5KHz
  if (!(scanLineCounter & 1)) {
    linePixel++;
  }
  if (TIM4_REG->CNT == 515) {
    videoOn = 0;
    frameNumber++;
  }
  if (TIM4_REG->CNT == 35) {
    videoOn = 1;
    linePixel = 0;
  }
}


// In STM32 Core, there is a linker conflict with HardwareTimer.cpp that defines the very same function...
// MUST include build_opt.h with this sketch just with the line below
// -DHAL_TIM_MODULE_ONLY
void TIM1_CC_IRQHandler(void)  {
  TIM1_REG->SR &= ~2;
  sendScanLine();
}


// starts timers for generating VGA sinal from pins A9 (VGA HSync) and B6 (VGA Vsync) for 640x480@60Hz
void video_init(uint8_t flashFont) {

  RCC_REG->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN;
  RCC_REG->APB1ENR |= RCC_APB1ENR_TIM4EN;
  RCC_REG->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
  GPIOA_REG->CRH = (GPIOA_REG->CRH & 0xFFFFFF0F) | 0x000000B0;
  GPIOB_REG->CRL = (GPIOB_REG->CRL & 0xF0FFFFFF) | 0x0B000000;
  GPIOC_REG->CRH = 0x33333333;

  TIM1_REG->CR1 = 0x80;
  TIM1_REG->CR2 = 0x20;
  TIM1_REG->CCER = 0x10;
  TIM1_REG->PSC = 0;
  TIM1_REG->CNT = 0;
  TIM1_REG->ARR = 2287;
  TIM1_REG->CCR2 = 275;
  TIM1_REG->CCR1 = flashFont ? 10 : 135;
  TIM1_REG->CCMR1 = 0x7800;
  
  TIM4_REG->CR1 = 0x80;
  TIM4_REG->CCER = 0x1;
  TIM4_REG->PSC = 0;
  TIM4_REG->ARR = 524;
  TIM4_REG->CNT = 0;
  TIM4_REG->CCR1 = 1;
  TIM4_REG->CCMR1 = 0x0078;
  TIM4_REG->SMCR = 0x07;

#ifdef ARDUINO_ARCH_STM32F1  // Roger's BluePill Core https://github.com/rogerclarkmelbourne/Arduino_STM32
  timer_attach_interrupt(&timer1, TIMER_CH1, sendScanLine);
#endif

#ifdef ARDUINO_ARCH_STM32  // Arduino_Core_STM32 Core https://github.com/stm32duino/Arduino_Core_STM32
  NVIC_EnableIRQ(TIM1_CC_IRQn);
  TIM1_REG->BDTR = 0x8000;
  TIM1_REG->DIER = 2;
  TIM1_REG->CCR1 = flashFont ? 40 : 165;
#endif

  TIM4_REG->CR1 |= 0x1;
  TIM1_REG->CR1 |= 0x1;
}

// stops timers and VGA sinal
void video_end() {
  TIM1_REG->CR1 &= ~0x1;
  TIM4_REG->CR1 &= ~0x1;
}

