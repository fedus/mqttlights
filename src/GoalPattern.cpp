/*
 GoalPattern.cpp - Goal driven pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "GoalPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

GoalPattern::GoalPattern(): Pattern() {
  // Empty constructor
}

FairyState GoalPattern::setup(FairyState &fstate) {
  // Set up base values for the pattern
  fstate.goal = fstate.curr;
  return Pattern::setup(fstate);
}

FairyState GoalPattern::getNext(FairyState &fstate) {
  if (fstate.curr != fstate.goal) {
    //int _step = (fstate.curr < fstate.goal) ? (int) fstate.step : -((int)fstate.step);
    int _step = (fstate.curr < fstate.goal) ? 1 : -1;
    if ((_step < 0 && (fstate.curr < (fstate.goal - _step))) || (_step > 0 && (fstate.curr > (fstate.goal - _step)))) {
      fstate.curr = fstate.goal;
    }
    else {
      fstate.curr += _step;
    }
  } else {
    custom_delay = 0;
  }
  return fstate;
}

void GoalPattern::setCustomDelay(unsigned long custom_delay) {
  this->custom_delay = custom_delay;
}
