/*
 BumpPattern.cpp - Bump pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "BumpPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

BumpPattern::BumpPattern(): Pattern() {
  // Empty constructor
}

FairyState BumpPattern::setup(FairyState &fstate) {
  fstate.curr = fstate.min;
  fstate.goal = fstate.min;
  return Pattern::setup(fstate);
}

FairyState BumpPattern::getNext(FairyState &fstate) {
  // Randomly determine if we want to bump. Initial chance with step=1 is 1/5 chance for bump.
  uint8_t scale = 5*fstate.step;
  if(random(0, scale) > scale-2) {
    fstate.curr += random(0,(fstate.max-fstate.curr)/2);
  }

  counter++;
  if (counter >= 100) {
    cycle(fstate);
    counter = 0;
  }
  fstate.goal = fstate.min;
  return fstate;
}
