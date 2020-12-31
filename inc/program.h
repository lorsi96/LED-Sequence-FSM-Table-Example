/*=============================================================================
 * Copyright (c) 2020, Lucas Orsi <lorsi@itba.edu.ar>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2020/12/30
 * Version: 0.1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __LORSI_H__
#define __LORSI_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Publis constants.]===================================================*/
#define DEBOUNCE_DELAY_MS 100
#define LONG_BLINK_MS 750
#define SHORT_BLINK_MS 150
#define ROLL_DELAY_MS 100

/*=====[Definitions of public data types]====================================*/

/** Program possible states. */
typedef enum states {
   ROLLING, // LEDs individual blinking in sequence.
   BLINKING_ALL, // All LEDs turned on.
} State;

/** Events involve in the program execution. */
typedef enum events {
   NONE,
   REQ_SHORT_BLINK,
   REQ_LONG_BLINK,
   REQ_CHANGE_DIRECTION,
   REQ_CHANGE_SPEED,
   END_BLINK,
   NEXT_LED_SEQUENCE,
} Event;

/** Alias for runnable functions. */
typedef void (*Action)();

/** 
 * Entries for a FSM table.
 * 
 * An entry contains information about which action to run and what state to
 * go to when a given event arrives on a given state.
 */
typedef struct fsmEntry {
   State state;
   Event event;
   Action action;
   State nextState;
} FsmEntry;

/*=====[Declaration of public constants]=====================================*/

/** 
 * Holds the different FSM entries that dictate, as a whole, the stateful 
 * behavior of the program. 
 */
extern FsmEntry fsmTable[];

/** Rolling Led constants. */
extern gpioMap_t LEDS_TO_ROLL[];

/*=====[Prototypes (declarations) of public functions]=======================*/

/**
 * @brief Turns on the next LED in the sequence.
 */
void rollOnce();

/**
 * @brief Turns all of the LEDs on for #SHORT_BLINK_MS. 
 */
void startShortBlink();

/**
 * @brief Turns all of the LEDs on for #LONG_BLINK_MS. 
 */
void startLongBlink();

/** 
 * @brief Does nothing. 
 */
void doNothing();

/** 
 * @brief Puts the system into a known/safe state if an unexpected turn of 
 *  events occurs. 
 */
void errorCallback();

/**
 * @brief Roll leds in the oposite direction as they're currently rolling.
 * 
 * @note Calling this method works even if the program is not currently rolling 
 *  the LEDs, meaning that whenever the system goes back into the rolling state 
 *  it will start shifting the LEDs in the oposite direction. 
 */
void toggleDirection();

/**
 * @brief Changes rolling speed randomly.
 */
void changeSpeedRandomly();


/*=====[Prototypes (declarations) of FSM functions]=========================*/
/** 
 * @brief Returns any relevant if event if any occurred.
 * 
 * @note This method is called on every spin of the FSM to check for events.
 */
Event getEvent();

/**
 * @brief Spins the FSM once.
 */
void spinFsm(Event event);

/*=====[Prototypes (declarations) of private functions]======================*/
static void turnAllLeds_(uint8_t value);
static inline void turnAllLedsOff_();
static inline void turnAllLedsOn_();
static inline uint8_t getBit_(uint8_t n, uint8_t k) ;


/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __LORSI_H__ */
