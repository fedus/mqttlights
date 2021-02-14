/*
 Pattern.cpp - Simple pattern base class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include <functional>
#include "Pattern.h"
#include "FairyCommon.h"

Pattern::Pattern() {
  // Empty constructor
}

FairyState Pattern::setup(FairyState &fstate) {
  counter = 0;
  custom_delay = 0;
  fstate.cycle = 0;
  return fstate;
}

uint8_t Pattern::cycle(FairyState &fstate) {
  fstate.cycle++;
  if(cycle_callback != NULL) cycle_callback();
  return fstate.cycle;
}

void Pattern::setCycleCallback(const CycleCallback &callback) {
  this->cycle_callback = callback;
}

unsigned long Pattern::getCustomDelay() {
  return custom_delay;
}
