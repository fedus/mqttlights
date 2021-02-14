/*
 SparklePattern.cpp - Sparkle pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "SparklePattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

SparklePattern::SparklePattern(): Pattern() {
  // Empty constructor
}

FairyState SparklePattern::setup(FairyState &fstate) {
  fstate.curr = fstate.min;
  fstate.goal = fstate.min;
  return Pattern::setup(fstate);
}

FairyState SparklePattern::getNext(FairyState &fstate) {
  // Randomly determine if we want to bump. Initial chance with step=1 is 1/5 chance for sparkle.
  fstate.curr = fstate.min;
  fstate.goal = fstate.min;

  uint8_t scale = 5*fstate.step;
  if(random(0, scale) > scale-2) {
    fstate.curr = random(fstate.min+1, fstate.max+1);
  }

  counter++;
  if (counter >= 100) {
    cycle(fstate);
    counter = 0;
  }

  return fstate;
}
