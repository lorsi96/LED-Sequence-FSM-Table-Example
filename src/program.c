/*=============================================================================
 * Copyright (c) 2020, Lucas Orsi <lorsi@itba.edu.ar>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2020/12/30
 * Version: 0.1
 *===========================================================================*/
#include "program.h"
#include "sapi.h"

/*=====[Program Constants]==========================*/

/** Delay Constants. */
#define DEBOUNCE_DELAY_MS 100
#define LONG_BLINK_MS 750
#define SHORT_BLINK_MS 150
#define MIN_ROLL_DELAY_MS 10
#define MAX_ROLL_DELAY_MS 300

/** Rolling Led constants. */
gpioMap_t LEDS_TO_ROLL[] = {
   LED1, LED2, LED3, LEDB
};
uint8_t LEDS_LEN = sizeof(LEDS_TO_ROLL) / sizeof(gpioMap_t);

/*=====[Definition of the FSM ]==========================*/
FsmEntry fsmTable[] = {
   {ROLLING, REQ_SHORT_BLINK, startShortBlink, BLINKING_ALL},
   {ROLLING, REQ_LONG_BLINK, startLongBlink, BLINKING_ALL},
   {ROLLING, REQ_CHANGE_DIRECTION, toggleDirection, ROLLING},
   {ROLLING, REQ_CHANGE_SPEED, changeSpeedRandomly, ROLLING},
   {ROLLING, END_BLINK, errorCallback, ROLLING},
   {ROLLING, NEXT_LED_SEQUENCE, rollOnce, ROLLING},
   {BLINKING_ALL, REQ_SHORT_BLINK, doNothing, BLINKING_ALL},
   {BLINKING_ALL, REQ_LONG_BLINK, doNothing, BLINKING_ALL},
   {BLINKING_ALL, REQ_CHANGE_DIRECTION, toggleDirection, BLINKING_ALL},
   {BLINKING_ALL, REQ_CHANGE_SPEED, changeSpeedRandomly, BLINKING_ALL},
   {BLINKING_ALL, END_BLINK, doNothing, ROLLING},
   {BLINKING_ALL, NEXT_LED_SEQUENCE, doNothing, BLINKING_ALL},
};

/*=====[Global variables]================================*/

/** Seed for random LFSR LED sequencing. */
uint8_t lfsr = 0b11001010;

/** Time between LED blinks for the current sequence. */
uint16_t currentRollDelay = MAX_ROLL_DELAY_MS;

/** Rolling Direction. */
static bool rollRight = true;

/** Non-blocking delay managers. */
static delay_t shortBlinkDelay;
static delay_t longBlinkDelay; 
static delay_t rollDelay;
static delay_t debounceDelay;

/** FSM state. */
static State state = ROLLING;

/*=====[Public method definitions]==========================*/

void doNothing() {}

void errorCallback() {}

void toggleDirection() {
   rollRight = !rollRight;
}

void startShortBlink() {
   turnAllLedsOn_();
   delayConfig(&shortBlinkDelay, SHORT_BLINK_MS);
}

void startLongBlink() {
   turnAllLedsOn_();
   delayConfig(&longBlinkDelay, LONG_BLINK_MS);
}

void rollOnce() {
   static int8_t ledToRoll = 0;
   turnAllLedsOff_();
   ledToRoll = (ledToRoll + 1) % ((int8_t)LEDS_LEN);
   gpioWrite(
      LEDS_TO_ROLL[
         rollRight ? ledToRoll : (((int8_t)LEDS_LEN) - ledToRoll - 1)
      ], 
      ON);
}

void changeSpeedRandomly() {
   lfsr =  (((lfsr << 1) | (getBit_(lfsr, 5) ^ (getBit_(lfsr, 6)) ^ (getBit_(lfsr, 7)))));
   currentRollDelay = MIN_ROLL_DELAY_MS + (MAX_ROLL_DELAY_MS * (uint16_t)lfsr / 0xFF);
}

/*=====[FSM Management Functions]============================*/

void spinFsm(Event event) {
   uint8_t count;
   for(count=0; count<(sizeof(fsmTable)/sizeof(FsmEntry)); count++) {
      if(fsmTable[count].state == state && fsmTable[count].event == event) {
         fsmTable[count].action();
         state = fsmTable[count].nextState;
      }
   }
}

Event getEvent() {
   if(state == BLINKING_ALL && delayRead(&longBlinkDelay)) {
      return END_BLINK;
   }
   if(state == ROLLING && delayRead(&rollDelay)) {
      delayWrite(&rollDelay, currentRollDelay);
      return NEXT_LED_SEQUENCE;
   }
   if(delayRead(&debounceDelay)) {
      if(!gpioRead(TEC2)) {
         return REQ_LONG_BLINK;
      }
      if(!gpioRead(TEC3)) {
         return REQ_SHORT_BLINK;
      }
      if(!gpioRead(TEC4)) {
         return REQ_CHANGE_DIRECTION;
      }
      if(!gpioRead(TEC1)) {
         return REQ_CHANGE_SPEED;
      }
   }
   return NONE;
}

/*=====[Private method definitions]==========================*/

static void turnAllLeds_(uint8_t value) {
   uint8_t cnt;
   for(cnt=0; cnt<LEDS_LEN; cnt++) {
      gpioWrite(LEDS_TO_ROLL[cnt], value);
   }
}

static inline void turnAllLedsOff_() {
   turnAllLeds_(OFF);
}

static inline void turnAllLedsOn_() {
   turnAllLeds_(ON);
}

static inline uint8_t getBit_(uint8_t n, uint8_t k) {
   return ((n >> k)  & 0x01);
}


/*=====[Main Loop]===========================================*/
int main(void)
{
   /* Setup. */
   boardInit();
   delayConfig(&debounceDelay, DEBOUNCE_DELAY_MS);
   /* Loop. */
   for(;;) {
      spinFsm(getEvent());
      delay(30); // We don't need higher a frequency,
                 // human vision won't note any difference.
   }
   return 0;
}
