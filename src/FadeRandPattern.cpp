/*
 FadeRandPattern.cpp - Random hi/lo fading pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "FadeRandPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

FadeRandPattern::FadeRandPattern(): Pattern() {
  // Empty constructor
}

FairyState FadeRandPattern::setup(FairyState &fstate) {
  // Set up base values for the pattern
  highGoal = true;  // Reset to default
  fstate.goal = fstate.curr; // To force calculation of new goal on first getNext()
  return Pattern::setup(fstate);
}

FairyState FadeRandPattern::getNext(FairyState &fstate) {
  if(fstate.curr == fstate.goal) {
    if(highGoal) {
      // Goal reached! Was fading UP, so now need to fade DOWN
      uint8_t new_diff = random(0, fstate.curr-fstate.min);
      fstate.goal = fstate.min + new_diff;
      highGoal = false;
    }
    else {
      // Goal reached! Was fading DOWN, so now need to fade UP!
      uint8_t new_diff = random(0, fstate.max-fstate.curr);
      fstate.goal = fstate.max - new_diff;
      highGoal = true;
    }
    // If the following statement is true, lights have been faded down 4 times (1=round1, 2=round2, 3=round3, 4=round4)
    // since we hit the counter already during the first getNext() due to setup (goal = curr)
    if (counter >= 4) cycle(fstate);
    else counter++;
  }
  return fstate;
}
