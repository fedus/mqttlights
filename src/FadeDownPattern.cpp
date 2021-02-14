/*
 FadeDownPattern.cpp - Fade Down pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "FadeDownPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

FadeDownPattern::FadeDownPattern(): Pattern() {
  // Empty constructor
}

FairyState FadeDownPattern::setup(FairyState &fstate) {
  // Set up base values for the pattern
  fstate.curr = fstate.max;
  fstate.goal = fstate.min;
  return Pattern::setup(fstate);
}

FairyState FadeDownPattern::getNext(FairyState &fstate) {
  if(fstate.curr == fstate.goal) {
    fstate.curr = fstate.max;
    fstate.goal = fstate.min;
    // If the following statement is true, lights have been faded down 4 times (0=round1, 1=round2, 2=round3, 3=round4)
    if (counter >= 3) cycle(fstate);
    else counter++;
  }
  return fstate;
}
