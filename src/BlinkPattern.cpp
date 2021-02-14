/*
 BlinkPattern.cpp - Blink pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "BlinkPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

BlinkPattern::BlinkPattern(): Pattern() {
  // Empty constructor
}

FairyState BlinkPattern::setup(FairyState &fstate) {
  // Set up base values for the pattern
  fstate.curr = fstate.max;
  fstate.goal = fstate.max;
  return Pattern::setup(fstate);
}

FairyState BlinkPattern::getNext(FairyState &fstate) {
  if ((counter % 25) == 0) toggle(fstate);
  counter++;
  if (counter >= 100) {
    cycle(fstate);
    counter = 0;
  }
  return fstate;
}

void BlinkPattern::toggle(FairyState &fstate) {
  if (fstate.goal == fstate.min) {
    fstate.goal = fstate.max;
    fstate.curr = fstate.max;
  }
  else {
    fstate.goal = fstate.min;
    fstate.curr = fstate.min;
  }
}
