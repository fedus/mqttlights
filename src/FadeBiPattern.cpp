/*
 FadeBiPattern.cpp - In/Out fading pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "FadeBiPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

FadeBiPattern::FadeBiPattern(): Pattern() {
  // Empty constructor
}

FairyState FadeBiPattern::setup(FairyState &fstate) {
  // Set up base values for the pattern
  fstate.curr = fstate.min;
  fstate.goal = fstate.max;
  return Pattern::setup(fstate);
}

FairyState FadeBiPattern::getNext(FairyState &fstate) {
  if(fstate.curr == fstate.goal) {
    if(abs(fstate.goal - fstate.min) < abs(fstate.goal - fstate.max)) {
      // Lights reached min goal, setting new max goal.
      fstate.goal = fstate.max;
      // If the following statement is true, lights have been faded up and down twice (starting at curr=min, goal=max in setup())
      if (counter > 0) cycle(fstate);
      else counter++;
    }
    else {
      // Lights reached max goal, setting new min goal.
      fstate.goal = fstate.min;
    }
  }
  return fstate;
}
