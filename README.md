# FSM Table Example

## How to run
This project follows the structure of a `new program` inside the [firmware_v3](https://github.com/epernia/firmware_v3/) project structure.

## Description
The program turns on, in sequence and one at a time, LED1, LED2, LED3 and LEDB of an EDU-CIAA-NXP development board.
If TEC keys are pressed some events are triggered as follows:
- TEC1: Pesudo-randomly changes the speed in which the sequence is run.
- TEC2: All leds are turned on for a long period of time.
- TEC3: All leds are turned on for a short period of time.
- TEC4: The led sequence is inverted (the led that turned on first becomes the last one).