/*
 FadeUpPattern.cpp - Fade Up pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "FadeUpPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

FadeUpPattern::FadeUpPattern(): Pattern() {
  // Empty constructor
}

FairyState FadeUpPattern::setup(FairyState &fstate) {
  // Set up base values for the pattern
  fstate.curr = fstate.min;
  fstate.goal = fstate.max;
  return Pattern::setup(fstate);
}

FairyState FadeUpPattern::getNext(FairyState &fstate) {
  if (fstate.curr == fstate.goal) {
    fstate.curr = fstate.min;
    fstate.goal = fstate.max;
    // If the following statement is true, lights have been faded up 4 times (0=round1, 1=round2, 2=round3, 3=round4)
    if (counter >= 3) cycle(fstate);
    else counter++;
  }
  return fstate;
}
