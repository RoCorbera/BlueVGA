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
#include "font.h"            // imports an ASCII Flash font bitmap
#include "graph_bitmaps.h"   // bitmap of sprites (invaders, explosions, player, lasers, etc)
#include "bluebitmap.h"      // functions for drawing bitmaps and sprites in the screen

// creates a BlueVGA object using RAM Tiles as bitmap for graphical usage
// USE_RAM means that we shall use an empty RAM space for 256 tiles of 8x8 pixels
BlueVGA vga(USE_RAM);

// 0 = blank space ' '         1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22
const uint8_t gameChars[] = { 'H', 'I', '-', 'S', 'C', 'O', 'R', 'E', '<', '>', 'D', 'T', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const uint8_t screenLine1[] = { 0, 4, 5, 6, 7, 8, 9, 14, 10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 4, 5, 6, 7, 8, 9, 15, 10, 0 };
const uint8_t screenLine2[] = { 0, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const uint8_t screenLine3[] = { 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 7, 8, 11, 2, 12, 0, 13, 13, 0, 0 };

#define INV_COLS     8
#define INV_ROWS     5
#define INV_LASERS   5  

void setup() {
  ScreenSetup();    // performs initial screen character drawing
  Animation();      // forever loop...
}


void loop() {
  vga.waitVSync(); // wait for the VBLANK
  // no need to code anything here... it's just a function that is called for ever
  // setup() can do it all, with a for_ever_loop such as in Animation()
}


typedef struct {
  uint8_t x, y, state;
} Invader;

typedef struct {
  uint8_t x, y, state, column;
} InvaderBomb;

// Invaders States and Bitmap Frames
enum { INV1_A = 0, INV1_B, INV2_A, INV2_B, INV3_A, INV3_B, EXPLODING, DEAD = EXPLODING + 30 };

// Player states
enum { PLAYER_ALIVE = 0, PLAYER_BLAST, PLAYER_DEAD = PLAYER_BLAST + 12};

// 4 diferent Invader Lasers or Bombs - Bitmap Frames and Blast States
enum {
  LASER1A = 0, LASER1B, LASER1C, LASER1D, LASER2A, LASER2B, LASER2C, LASER2D, LASER3A, LASER3B, LASER3C, LASER3D,
  LASER_IDLE,
  LASER_BLAST, LASER_BLAST_END = LASER_BLAST + 45
};

// player laser states
enum { PLAYER_LASER_OFF = 0, PLAYER_LASER_ON, PLAYER_LASER_BLAST_ON, PLAYER_LASER_BLAST_OFF = PLAYER_LASER_BLAST_ON + 10 };

// State used for controlling which Invader Column has released its bomb
enum { FIRE_OFF = 0, FIRE_ON};

void setScore(uint32_t sc, bool isHiScore) {
  int8_t offsetX = 4;
  uint8_t xPos = isHiScore ? 11 : 3;
  bool printAtLeastZero = true;
  do {
    uint8_t digit = sc % 10;
    if (sc > 0) vga.setTile(xPos + offsetX, 1, 13 + digit);  // 13 = '0'
    else vga.setTile(xPos + offsetX, 1, 13);   // 13 = '0'
    sc /= 10;
    offsetX--;
  } while (offsetX >= 0);
}

void Animation (void) {

  static uint8_t sprX = 5, sprY = 40;


  static int8_t invadersDir = 1, ufoDir = 1;    // starts on 1
  static bool ufoStart = false;
  static uint8_t ufoX = 0, ufoFrame = 0;
  static uint8_t invadersNum = 0;
  static uint8_t numInvadersAlive = INV_COLS * INV_ROWS;
  static uint8_t invaderRight = 0, invaderLeft = INV_COLS - 1;
  static uint8_t bottomInvaders[INV_COLS]; 
  static bool shallMoveDown = false, theyInvadedUs = false;

  static InvaderBomb lasers[INV_LASERS];
  static uint8_t InvadersExplosions = 0, bombInterval = 0;
  static uint8_t bombInvCol[INV_COLS] = { FIRE_OFF };

  static uint32_t score = 0, hiScore = 0;

  static uint8_t playerX = 10;   // 10 .. 224 - 13 - 10
  static int8_t playerDX = 1;
  static uint8_t playerState = PLAYER_ALIVE;
  static uint8_t playerLaserX = 0, playerLaserY = 0, playerLaserState = PLAYER_LASER_OFF;

  static Invader enemies[INV_COLS * INV_ROWS];

  for (uint8_t row = 0; row < INV_ROWS; row++)
    for (uint8_t col = 0; col < INV_COLS; col++) {
      uint8_t invIdx = row * INV_COLS + col;
      enemies[invIdx].x = sprX + col * 16;
      enemies[invIdx].y = sprY + (INV_ROWS - 1 - row) * 12;
      if (row < 2) enemies[invIdx].state = INV1_A;
      else if (row < 4) enemies[invIdx].state = INV2_A;
      else enemies[invIdx].state = INV3_A;
    }

  // all enemy lasers ready!
  for (uint8_t i = 0; i < INV_LASERS; i++) {
    lasers[i].state = LASER_IDLE;
  }
  for (uint8_t i = 0; i < INV_COLS; i++) bottomInvaders[i] = i;

  // Creating the sprites for de animation - most of them have more than one frame bitmap by individual animation
  BlueBitmap invaderBitmap;
  invaderBitmap.setBitmap(16, 8, (uint8_t *)invaderBitmaps);

  BlueBitmap invFireBlastBitmap;
  invFireBlastBitmap.setBitmap(8, 8, (uint8_t *)invFireBlast);

  BlueBitmap playerFireBlastBitmap;
  playerFireBlastBitmap.setBitmap(8, 8, (uint8_t *)playerFireBlast);

  BlueBitmap ufoBitmap;
  ufoBitmap.setBitmap(16, 8, (uint8_t *)ufoBitmaps);

  BlueBitmap playerBitmap;
  playerBitmap.setBitmap(16, 8, (uint8_t *)playerBitmaps);

  BlueBitmap invLaserBitmap;
  invLaserBitmap.setBitmap(8, 8, (uint8_t *)invLaser);

  BlueBitmap playerLaserBitmap;
  playerLaserBitmap.setBitmap(8, 8, (uint8_t *)playerLaser);

  setScore(0, false);


  while (true) {   // forever ... sort of replaces loop()

    vga.waitVSync(); // waits for the VBLANK, right after the screen has been drawn in the VGA monitor

    /*

       Shields are RAM bitmaps that must have their pixels correctly set to zero

    */

    // clears invader bombs before RESETING tiles for drawing the screen. This makes sure Bottom Line is correctly "marked"
    for (uint8_t i = 0; i < INV_LASERS; i++)
      if (lasers[i].state < LASER_IDLE) // && lasers[i].y > (VRAM_HEIGHT - 1 - 2) * 8 )
        invLaserBitmap.drawBitmap8(lasers[i].x, lasers[i].y, lasers[i].state, false);   // erases all invader bombs
      else if (lasers[i].state > LASER_IDLE)
        invFireBlastBitmap.drawBitmap8(lasers[i].x - 3, lasers[i].y, 0, false);         // erases all invader bombs

    // clears invader of this cycle to mark shield
    if (enemies[invadersNum].state < DEAD) invaderBitmap.drawBitmap8(enemies[invadersNum].x, enemies[invadersNum].y, enemies[invadersNum].state, false);

    // unmarks the shield with the laser
    if (playerLaserState == PLAYER_LASER_ON) playerLaserBitmap.drawBitmap8(playerLaserX, playerLaserY, 0, false);

    // clears player laser explosion, in case those bitmaps are over Shield area (it can generate artifacts and populate its bitmap)
    if (playerLaserState >= PLAYER_LASER_BLAST_ON) playerFireBlastBitmap.drawBitmap8(playerLaserX, playerLaserY, 0, false);

    // Erases game space, leaving top and botom screen intact - sort of general RESET for all tiles used to draw the game screen (expect special case SHIELD)
    vga.setTileRowsFast(2, VRAM_HEIGHT - 1 - 2, 0);  // higher than Shiled level...

    // Erases all the RAM Tiles used in drawing the bitmaps and sprites - fresh start
    BlueBitmap::eraseRamTiles();

    /*

       Moving actors and logic for firing. UFO start

    */

    // moves invaders one by one
    if (enemies[invadersNum].state < EXPLODING && InvadersExplosions == 0) {     // Invaders only move when their explosions have ended...
      // is this invader at the screen lateral edge?
      if (shallMoveDown) enemies[invadersNum].y += 8;                            // moves it down when at the edge
      enemies[invadersNum].x += 2 * invadersDir;                                 // moves it to the side in a direction
      if (enemies[invadersNum].state & 1) enemies[invadersNum].state--;          // changes sprite bitmap to make it "walk"
      else enemies[invadersNum].state++;
    }

    // which invaders are alive?
    // adjusts border limits on each side
    for (uint8_t b = 0; b < INV_COLS; b++) {
      while (bottomInvaders[b] < INV_COLS * INV_ROWS && enemies[bottomInvaders[b]].state >= EXPLODING) bottomInvaders[b] += INV_COLS;
      if (bottomInvaders[b] < INV_COLS * INV_ROWS && enemies[bottomInvaders[b]].y > (VRAM_HEIGHT - 1 - 4) * 8) theyInvadedUs = true;  // marks invaders columns that have no invader to fire laser
    }
    // checks for the leftest invaders in the group
    for (uint8_t b = 0; b < INV_COLS; b++)
      if (bottomInvaders[b] < INV_COLS * INV_ROWS)  {
        invaderLeft = bottomInvaders[b];
        break;
      }
    // checks for the rightest invader in the group
    for (int8_t b = INV_COLS - 1; b >= 0; b--)
      if (bottomInvaders[b] < INV_COLS * INV_ROWS)  {
        invaderRight = bottomInvaders[b];
        break;
      }

    // Checks if there are free invaders to fire that haven't still fired
    uint8_t freeLaserIdx = 255;
    for (uint8_t l = 0; l < INV_LASERS - numInvadersAlive / 10; l++)
      if (lasers[l].state == LASER_IDLE) {
        freeLaserIdx = l;
        break;
      }

    // random invader fire routine
    if (bombInterval) bombInterval--;
    if (freeLaserIdx != 255 && !bombInterval) {
      if (random(100) > 60 && random(100) > 80 - numInvadersAlive) {
        // closer invader to the palyer
        uint8_t minDist = 255;
        uint8_t col = 255;
        uint8_t startingCol = 0;
        if (random(100) > 40) startingCol = random(INV_COLS);       // inserts some randomic decision factor...
        for (uint8_t b = startingCol; b < INV_COLS; b++) {
          if (bottomInvaders[b] >= INV_COLS * INV_ROWS) continue;   // invader column OK and has not fired yet ....
          if (bombInvCol[b] != FIRE_OFF) continue;                  // invader column OK and has not fired yet ....
          int16_t dist = enemies[bottomInvaders[b]].x - playerX;
          if (dist < 0) dist = -dist;
          if (minDist > dist) {
            minDist = dist;
            col = b;
            if (random(100) > 90) break;  // inserts some randomic decision factor...
          }
        }
        if (col != 255) {  // is there an invader that will fire?
          // set the bomb on and assigns a bitmap to it
          bombInvCol[col] = FIRE_ON;
          lasers[freeLaserIdx].x = enemies[bottomInvaders[col]].x + 7;
          lasers[freeLaserIdx].y = enemies[bottomInvaders[col]].y + 12;
          lasers[freeLaserIdx].column = col;
          lasers[freeLaserIdx].state = random(3) * 4;
          bombInterval = 35;
        }
      }
    }


    // Player firing back animation - random!
    if (playerLaserState ==  PLAYER_LASER_OFF && playerState == PLAYER_ALIVE && random(100) > 45 && random(100) > 70 ) {
      playerLaserX = playerX;
      playerLaserY = VRAM_HEIGHT * 8 - 24 - 8;
      playerLaserState = PLAYER_LASER_ON;
    }


    // moves invader bombs
    for (uint8_t i = 0; i < INV_LASERS; i++)
      if (lasers[i].state < LASER_IDLE) {
        lasers[i].y += 1;
        // laser animation
        if (!(vga.getFrameNumber() & 7)) {
          uint8_t laserStep = lasers[i].state & 3;
          uint8_t laserKind = lasers[i].state - laserStep;
          lasers[i].state = laserKind + ((laserStep + 1) & 3);
        }
        // checks laser reaching screen bottom
        if (lasers[i].y == (VRAM_HEIGHT - 1 - 4 - 2) * 8 + 39) {
          lasers[i].state = LASER_BLAST;
        }
      }


    // moves player laser
    if (playerLaserState == PLAYER_LASER_ON) {
      playerLaserY -= 4;
      if (playerLaserY == 16) playerLaserState = PLAYER_LASER_BLAST_ON;
    }
    if (playerLaserState >= PLAYER_LASER_BLAST_ON) {
      playerLaserState++;
      if (playerLaserState == PLAYER_LASER_BLAST_OFF) playerLaserState = PLAYER_LASER_OFF;
    }


    // simulating UFO
    if (!ufoX && random(100) > 90) { // ufoX == 0 means it is not in the screen...
      ufoFrame = 0;
      if (ufoDir == 1) {
        ufoDir = -1;
        ufoX = VRAM_WIDTH * 8 - 16 - 8;
      } else {
        ufoDir = 1;
        ufoX = 8;
      }
    }


    /*
       Collisions
    */


    // draws shileds in their current state  --- before testing bomb pixels against it
    for (uint8_t s = 0; s < 3; s++)
      for (uint8_t y = 0; y < 3; y++)
        for (uint8_t x = 0; x < 3; x++)
          vga.setTile(x + shiledPosX[s], y + VRAM_HEIGHT - 1 - 4 - 2,  sizeof(gameChars) + 1 + x + 3 * y + 9 * s);

    // checks if invader bombs hits shield or player
    for (uint8_t i = 0; i < INV_LASERS; i++) {

      if (lasers[i].state < LASER_IDLE && lasers[i].y > (VRAM_HEIGHT - 1 - 4 - 3) * 8) {  // lower than shield Y level

        // hit player ?
        if (playerState == PLAYER_ALIVE && (lasers[i].y > (VRAM_HEIGHT - 1 - 3) * 8 + 6) && playerX + 12 >= lasers[i].x + 2 &&  lasers[i].x >= playerX ) {
          playerState = PLAYER_BLAST;
          lasers[i].state = LASER_IDLE;             // bomb is off
          bombInvCol[lasers[i].column] = FIRE_OFF;  // releases that column to fire again
        }

        // hit shields?
        // get the tile position in the screen for (upper) center of laser bitmap
        uint8_t xTile = lasers[i].x >> 3;
        uint8_t yTile = (lasers[i].y + 3) >> 3;
        uint16_t tileIdx = vga.getTile(xTile, yTile);
        if (!tileIdx) { // nothing hit, but we must check right bitmap edge as well
          xTile = (lasers[i].x + 2) >> 3;
          tileIdx = vga.getTile(xTile, yTile);
        }
        if (tileIdx >= sizeof(gameChars) + 1 && tileIdx < sizeof(gameChars) + 1  + 9 * 3) {
          // we are over shield tiles, let's test it
          uint8_t shieldTile[8];  // copy of the possible hit tile
          for (uint8_t p = 0; p < 8; p++) shieldTile[p] = *(BlueBitmap::ramFont + (tileIdx << 3) + p);
          invLaserBitmap.drawBitmap8(lasers[i].x, lasers[i].y, lasers[i].state, false);     // reset pixels of this invader bomb
          // compare pixels of the shiled tile after drawing the reset bomb pixels
          uint8_t hasHit = false;
          for (uint8_t p = 0; p < 8 && !hasHit; p++)
            if (shieldTile[p] != *(BlueBitmap::ramFont + (tileIdx << 3) + p)) hasHit = true;
          if (hasHit) {
            // marks the shield with the laser
            invLaserBitmap.drawBitmap8(lasers[i].x, lasers[i].y, lasers[i].state, false);
            // marks the shiled with laser bast
            invFireBlastBitmap.drawBitmap8(lasers[i].x - 3, lasers[i].y + 2 + random(3), 0, false);
            // releases the bitmap sprite object
            lasers[i].state = LASER_IDLE;             // bomb off
            bombInvCol[lasers[i].column] = FIRE_OFF;  // releases that column to fire again
          }
        }
      }
    }
    
    // checks if player Laser hit shileds
    if (playerLaserState == PLAYER_LASER_ON) {

      // shield collision
      // gets the tile position in the screen for (upper) center of laser bitmap
      uint8_t xTile = ((playerLaserX + 6) >> 3);
      uint8_t yTile = ((playerLaserY - 2) >> 3);
      uint16_t tileIdx = vga.getTile(xTile, yTile);

      if (tileIdx >= sizeof(gameChars) + 1 && tileIdx < sizeof(gameChars) + 1  + 9 * 3) {
        // we are over shield tiles, let's test it
        uint8_t shieldTile[8];  // copy of the possible hit tile
        for (uint8_t p = 0; p < 8; p++) shieldTile[p] = *(BlueBitmap::ramFont + (tileIdx << 3) + p);
        playerLaserBitmap.drawBitmap8(playerLaserX, playerLaserY - 2, 0, false);     // resets pixels of this invader bomb
        // compares pixels of the shiled tile after drawing the reset bomb pixels
        uint8_t hasHit = false;
        for (uint8_t p = 0; p < 8 && !hasHit; p++)
          if (shieldTile[p] != *(BlueBitmap::ramFont + (tileIdx << 3) + p)) hasHit = true;
        if (hasHit) {
          // marks the shield with the laser a bit lower
          playerFireBlastBitmap.drawBitmap8(playerLaserX + random(3), playerLaserY - 3 - random (3), 0, false);
          playerFireBlastBitmap.drawBitmap8(playerLaserX - random(3), playerLaserY - 1, 0, false);
          // releases the bitmap sprite object
          playerLaserState = PLAYER_LASER_OFF;   // player laser off
        }
      }

    }



    /*
       Moves and Draws all elements to the screen
    */


    // Moving and drawing UFO
    if (ufoX) {  // ufoX not ZERO means we have a UFO in the screen
      if (!(vga.getFrameNumber() & 1)) {       // defines velocity of UFO in frames/second  --> 2 frames/s
        ufoX += ufoDir;                        // moves UFO to new position
      }
      if (!(vga.getFrameNumber() & 3)) {       // defines change of UFO frames  --> 4 frames/s
        ufoFrame = (ufoFrame + 1) & 1;         // defines UFO bitmap frame change --> 2 frames
      }
      // did UFO reach screen edges?
      if (ufoX < 8 || ufoX > VRAM_WIDTH * 8 - 16 - 8) {
        ufoX = 0;
      } else {
        ufoBitmap.drawBitmap8(ufoX, 8 * 2, ufoFrame, true); // draws UFO
      }
    }

    // checks player STATE and SIMULATES moving from a side to the other
    uint8_t playerBitmapFrame = PLAYER_ALIVE;       // bitmap when playing
    if (playerState == PLAYER_ALIVE) {              // States are Alive = 0 and Blast = 1, 2, 3, 4, 5, 6, 7 ... counting frames and time
      playerX += playerDX;
      // playerX => 10 .. 224 - 13 - 10
      if (playerX > VRAM_WIDTH * 8 - 13 - 10 || playerX < 10) playerDX = -playerDX;
    } else {  // PLAYER_BLAST state forward
      playerBitmapFrame = (playerState & 1) + PLAYER_BLAST;    // Blast frames are 1 and 2 ...
      if (!(vga.getFrameNumber() & 7)) {                       // changes bitmap frame for Blast every 8 VGA frames (8/60 seconds)
        // time of death animation is variant on the actual FrameNumber of the event... PLAYER_DEAD defines this time
        if (++playerState == PLAYER_DEAD) playerState = PLAYER_ALIVE;
      }
    }
    // draws Player
    playerBitmap.drawBitmap8(playerX, VRAM_HEIGHT * 8 - 24, playerBitmapFrame);


    // draws ALL invaders on their places in the screen
    for (uint8_t i = 0; i < INV_COLS * INV_ROWS; i++) {
      uint8_t bitmapFrame = enemies[i].state;
      if (bitmapFrame >= EXPLODING && bitmapFrame < DEAD) bitmapFrame = EXPLODING;
      if (bitmapFrame < DEAD) invaderBitmap.drawBitmap8(enemies[i].x, enemies[i].y, bitmapFrame);
      // if Invader is exploding, makes its state advance in time (number of screen frames) up to reaching DEAD state, so it's not drawn anymore
      if (enemies[i].state >= EXPLODING && enemies[i].state != DEAD)
        if (++enemies[i].state == DEAD) {
          InvadersExplosions--;              // frees "semaphore" for Invaders explosions ONLY on the change of EXPLODING to DEAD...
          numInvadersAlive--;                // right after drawing invader explosion, decrements its counter
        }
    }

    // draws ALL invader bombs
    for (uint8_t i = 0; i < INV_LASERS; i++)
      if (lasers[i].state < LASER_IDLE)
        invLaserBitmap.drawBitmap8(lasers[i].x, lasers[i].y, lasers[i].state);
      else if (lasers[i].state >= LASER_BLAST) {
        lasers[i].state++;                                                           // laser goes into blast state
        if (lasers[i].state == LASER_BLAST_END) {
          invFireBlastBitmap.drawBitmap8(lasers[i].x - 3, lasers[i].y, 0, false);    // clears blast bitmap to create a "crater"
          lasers[i].state = LASER_IDLE;                                              // laser is off
          bombInvCol[lasers[i].column] = FIRE_OFF;                                   // releases that column to fire
        } else invFireBlastBitmap.drawBitmap8(lasers[i].x - 3, lasers[i].y, 0);      // draws invader bombs blast
      }

    // checks if player laser hits the UFO
    if (playerLaserState == PLAYER_LASER_BLAST_ON + 1 && playerLaserX + 6 >= ufoX + 2 && playerLaserX + 6 <= ufoX + 14) {  // just hit the top of the screen
      ufoX = 0; // ufo stops and it is not drawn any more
    }

    // checks if player laser hits any invaders or its bombs
    if (playerLaserState == PLAYER_LASER_ON) {
      // checks if hit any invader bomb
      for (uint8_t i = 0; i < INV_LASERS; i++) {
        if (lasers[i].state < LASER_IDLE &&
            lasers[i].y <= playerLaserY && lasers[i].y + 6 >= playerLaserY &&
            lasers[i].x <= playerLaserX + 6 && lasers[i].x + 2 >= playerLaserX + 6) {
          playerLaserState = PLAYER_LASER_BLAST_ON;
          lasers[i].state = LASER_BLAST;
        }
      }

      // checks if hit an invader
      const uint8_t invadersBitmapPadding [] = { 2, 2 + 11, 2, 2 + 10, 4, 4 + 7 };
      for (uint8_t i = 0; i < INV_COLS * INV_ROWS; i++) {
        if (enemies[i].state < EXPLODING &&
            enemies[i].y <= playerLaserY + 2 && enemies[i].y + 7 >= playerLaserY + 2 &&
            enemies[i].x + invadersBitmapPadding[enemies[i].state & ~1] <= playerLaserX + 6 &&
            enemies[i].x + invadersBitmapPadding[(enemies[i].state & ~1) + 1] >= playerLaserX + 6) {
          playerLaserState = PLAYER_LASER_OFF;
          enemies[i].state = EXPLODING;
          InvadersExplosions++;              // Adds 1 to the "Semaphore" of "There are Exploding Invaders" in the screen
        }
      }
    }

    // draws player Laser
    if (playerLaserState == PLAYER_LASER_ON) playerLaserBitmap.drawBitmap8(playerLaserX, playerLaserY);
    if (playerLaserState >= PLAYER_LASER_BLAST_ON) playerFireBlastBitmap.drawBitmap8(playerLaserX, playerLaserY);


    // checks end of game or next wave of invaders
    if (!numInvadersAlive || theyInvadedUs) break; // all invaders are dead... end of animation


    // testing using score printing - score is used to check states... a way to view execution status.
    if (BlueBitmap::getNextFreeTile() > hiScore) {
      hiScore = BlueBitmap::getNextFreeTile();
      setScore(hiScore, true);
    }
    // we can check number of RAM Tiles used along the animation. Going beyond 255 means it won't display right
    setScore(BlueBitmap::getNextFreeTile()/*vga.getFrameNumber()*/ /*numInvadersAlive*10*/, false);

    // next INVADER to move!
    if (InvadersExplosions == 0) {    // when there are exploding Invaders in the Screen... all of them FREEZE!

      // finds the next ALIVE Invader
      invadersNum++;
      while (enemies[invadersNum].state == DEAD && invadersNum  < INV_COLS * INV_ROWS) invadersNum++;

      if (invadersNum  == INV_COLS * INV_ROWS) {  // go around the invaders
        invadersNum = 0;
        // in case first invaders is dead... find next!
        while (enemies[invadersNum].state == DEAD) invadersNum = (invadersNum + 1) % (INV_COLS * INV_ROWS);

        shallMoveDown = false;                  // shall we move down?
        if ((enemies[invaderRight].x > 224 - 12 - 10 && invadersDir > 0) || (enemies[invaderLeft].x < 10 && invadersDir < 0)) {
          invadersDir = -invadersDir;                      // changes invaders direction
          shallMoveDown = true;                            // indicates that each invaders shall go down
        }
      }

      // invaders sound effect here... TBD!
    }
  }
}

void ScreenSetup (void) {

  // clears RAM Tiles and fills up the screen with tileID zero
  BlueBitmap::clearGraphScreen(vga.getColorCode(RGB_WHITE, RGB_BLACK));

  // Setting up main characters used in the game screen
  for (uint8_t i = 0; i < sizeof(gameChars); i++) BlueBitmap::copyFont2RamTile (gameChars[i], font8x8_icl8x8u, i + 1);

  // draws game screen text
  for (uint8_t x = 0; x < VRAM_WIDTH; x++) {
    vga.setTile(x, 0, screenLine1[x], vga.getColorCode(RGB_YELLOW, RGB_BLACK));
    vga.setTile(x, 1, screenLine2[x], vga.getColorCode(RGB_YELLOW, RGB_BLACK));
    vga.setTile(x, VRAM_HEIGHT - 1, screenLine3[x], vga.getColorCode(RGB_WHITE, RGB_BLACK));
  }

  uint8_t firstFreeTile = sizeof(gameChars) + 1 /* 0 = blank*/;          // reference index of the first usable tile for drawing pixels on the screen

  // draws shields
  for (uint8_t s = 0; s < 3; s++)
    for (uint8_t y = 0; y < 3; y++)
      for (uint8_t x = 0; x < 3; x++) {
        BlueBitmap::copyFont2RamTile (shieldFrames[x + 3 * y], shield, firstFreeTile + x + 3 * y + 9 * s);
        vga.setTile(x + shiledPosX[s], y + VRAM_HEIGHT - 1 - 4 - 2, firstFreeTile + x + 3 * y + 9 * s);
      }
  vga.setColorRegion(0, VRAM_HEIGHT - 1 - 4 - 2, VRAM_WIDTH - 1, VRAM_HEIGHT - 1 - 4, vga.getColorCode (RGB_GREEN, RGB_BLACK));

  firstFreeTile += 9 * 3;                         // adding 3 shileds of 3x3 tiles

  for (uint8_t x = 0; x < VRAM_WIDTH; x++) {
    BlueBitmap::copyFont2RamTile (0, bottomLine, firstFreeTile + x);
    vga.setTile(x, VRAM_HEIGHT - 1 - 1, firstFreeTile + x);
  }
  vga.setColorRegion(0, VRAM_HEIGHT - 1 - 1, VRAM_WIDTH - 1, VRAM_HEIGHT - 1 - 1, vga.getColorCode (RGB_GREEN, RGB_BLACK));  // Screen Bottom Line Space

  vga.setColorRegion(0, 2, VRAM_WIDTH - 1, 3, vga.getColorCode (RGB_RED, RGB_BLACK)); // UFO travel space :-)
  vga.setColorRegion(0, VRAM_HEIGHT - 1 - 2, VRAM_WIDTH - 1, VRAM_HEIGHT - 1 - 3, vga.getColorCode (RGB_CYAN, RGB_BLACK));  //Player Space

  firstFreeTile += VRAM_WIDTH;                    // adding the horizontal line at the end of the screen

  BlueBitmap::setFirstTile(firstFreeTile);
  BlueBitmap::setNextFreeTile(firstFreeTile);     // Tile 0 used as default background
  randomSeed(analogRead(PA0));
  vga.waitVSync(120); // wait for the VBLANK
}

