/*
 FairyCommon.h - Common declarations for FairyLights.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef FairyCommon_h
#define FairyCommon_h

#define CYCLE 4

typedef enum Mode
{
  STATIC,
  FADE_BI,
  FADE_UP,
  FADE_DOWN,
  FADE_RAND,
  BLINK,
  BUMP,
  SPARKLE,
  MIX,
};

typedef struct FairyState
{
 uint8_t curr;
 uint8_t prev;
 uint8_t goal;
 uint8_t max;
 uint8_t min;
 uint8_t step;
 uint8_t cycle;
 unsigned long delay;
 Mode mode;
 uint8_t mix_mode;
};

#endif
