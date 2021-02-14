/*
 FairyLights.cpp - Simple controller for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include <functional>
#include "cie1931.h"
#include "FairyLights.h"
#include "Pattern.h"
#include "NullPattern.h"
#include "GoalPattern.h"
#include "FadeBiPattern.h"
#include "FadeUpPattern.h"
#include "FadeDownPattern.h"
#include "FadeRandPattern.h"
#include "BlinkPattern.h"
#include "BumpPattern.h"
#include "SparklePattern.h"
#include "FairyCommon.h"
#if defined(ESP32)
#include <analogWrite.h>
#endif

FairyLights::FairyLights() {
  // Empty constructor
}

void FairyLights::setup(int pin) {
  // Constructor for default initial brightness setting
  setup(pin, STD_BRI, STD_MIN, STD_MAX, STD_STEP, STD_DELAY);
}

void FairyLights::setup(int pin, int brightness, int min, int max, int step, unsigned long delay) {
  // Constructor for custom initial brightness setting
  pinNumber = pin;
  previousMillis = 0;
  prevBri = -1;
  //fstate.mode = STATIC;
  setMode(STATIC);
  fstate.delay = delay;
  fstate.min = min;
  fstate.max = max;
  fstate.step = step;
  fstate.mix_mode = 0;    // Note that this is technically an invalid value, however it is fine to initialise it this way as it is checked and reset at the first CycleCallback call
  fstate.cycle = false;   // This is just a binary value indicating whenever a pattern has completed a full "cycle"
  initBrightness(brightness);
  // Sets correct pin mode
  pinMode(pinNumber, OUTPUT);

  // TESTING:
  /*
  GoalPattern *bla = (GoalPattern*) (patterns[STATIC]);
  bla->setup(&briCurr, &briGoal, &briMin, &briMax, &this->step);
  bla->getNext();
  */
  Pattern **patterns_ = patterns;
}

void FairyLights::initBrightness(int brightness) {
  // Initialise standard values for the initial brightness setting
  fstate.curr = brightness;
  fstate.prev = brightness;
  fstate.goal = brightness;
}

void FairyLights::applyBrightness() {
  // Applies a given brightness to the pin, if necessary.
  if (prevBri != fstate.curr) {
    prevBri = fstate.curr;
    int brightness_cie = cie[int(fstate.curr)];  // Fetches the corresponding value from the CIE table for linear brightness
    #if defined(ESP32)
    analogWrite(pinNumber, brightness_cie, 1024);
    #else
    analogWrite(pinNumber, brightness_cie);
    #endif
  }
}

void FairyLights::handle() {
  // We always use the goal pattern, since it is the basis for the static mode and is a helper for other patterns.
    /*Serial.print("** FROM HANDLE(): mix_mode:");
    Serial.print(fstate.mix_mode);
    Serial.print(" mode: ");
    Serial.println(fstate.mode);*/
  patt_goal.getNext(fstate);

  // Use the user-selected pattern.
  patterns[fstate.mode]->getNext(fstate);

  // set the new brightness
  applyBrightness();
}

void FairyLights::loop() {
  // Loop function that needs to be called in the standard Arduino loop
  currentMillis = millis();

  // Static mode might have its own delay (for fading)
  evaluated_delay = (fstate.mode == STATIC && patt_goal.getCustomDelay()) ? patt_goal.getCustomDelay() : fstate.delay;

  //Serial.println(patterns[fstate.mode]->getCustomDelay());

  if(currentMillis - previousMillis > evaluated_delay) {
    previousMillis = currentMillis;

    handle();
  }

}

uint8_t FairyLights::setGoal(int brightness, bool fade /*=false*/, bool savePrev /*=true*/, uint8_t custom_delay /*=0*/) {
  // Sets a new brightness "goal". This might imply an instantaneous brightness change, or a gradual one.
  brightness = constrain(brightness, INPUT_MIN, INPUT_MAX);   // Constrain value to min and max input
  if (savePrev == true) {
    // If savePref == true, we save the current brightness goal as the previous value to keep a simple history
    fstate.prev = fstate.goal;
  }
  if (fade == false) {
    // If an instantaneous change is desired, we have to set the current brightness equal to the goal
    fstate.curr = brightness;
  } else {
    patt_goal.setCustomDelay(custom_delay);
  }
  // Set the new brightness goal.
  fstate.goal = brightness;
  return fstate.goal;
}

uint8_t FairyLights::revertGoal(bool fade /*=false*/) {
  // Go back to previously saved value
  setGoal(fstate.prev, fade, false);
  return fstate.goal;
}

void FairyLights::cycleCallback() {
  // This is a callback that gets calle whenever a pattern has finished, but is only relevant if we are in mix mode to cycle through patterns.
  if(fstate.mode == MIX) {
    this->fstate.mix_mode++;
    // Our pattern array contains the goal pattern at pos 0 and the changing mix mode as last element, so we want to exclude those.
    if(fstate.mix_mode < 1 || fstate.mix_mode >= (sizeof(patterns)/sizeof(patterns[0]) - 1)) {
      fstate.mix_mode = 1;
    }
    patterns[MIX] = patterns[fstate.mix_mode];
    setupPattern(MIX);
    /*Serial.print("** FROM CALLBACK: Callback mix_mode:");
    Serial.print(fstate.mix_mode);
    Serial.print(" mode: ");
    Serial.println(fstate.mode);*/
  }
}

void FairyLights::setupPattern(Mode mode_value) {
  patterns[mode_value]->setup(fstate);
  //patterns[mode_value]->setCycleCallback(std::bind(&FairyLights::cycleCallback, *this));   // Alternative lambda: [this] () { this->cycleCallback(); }
}

Mode FairyLights::setMode(Mode mode_value) {
  // Sets a new mode
  fstate.mode = mode_value;
  setupPattern(fstate.mode);
  if(mode_value == STATIC) patt_goal.setup(fstate);    // Does not need callback and cannot reliably be setup using setupPattern
  else if (mode_value == MIX) {
    for(int i = 1; i < (sizeof(patterns)/sizeof(patterns[0]) - 1); i++) patterns[i]->setCycleCallback(std::bind(&FairyLights::cycleCallback, std::ref(*this)));   // Alternative lambda: [this] () { this->cycleCallback(); }
    cycleCallback();
  }
  return mode_value;
}

uint8_t FairyLights::bump(int bump_value) {
  // Bumps up the brightness by given amount.
  fstate.curr = constrain(fstate.curr + bump_value, INPUT_MIN, INPUT_MAX);
  return bump_value;
}

uint8_t FairyLights::setMax(int max_value) {
  // Set new maximum value. Cannot be higher than INPUT_MAX or lower than current user-set minimum + 5.
  fstate.max = constrain(max_value, fstate.min+5, INPUT_MAX);
  return fstate.max;
}

uint8_t FairyLights::setMin(int min_value) {
  // Set new minimum value. Cannot be lower than INPUT_MIN or higher than current user-set maximum - 5.
  fstate.min = constrain(min_value, INPUT_MIN, fstate.max-5);
  return fstate.min;
}

uint8_t FairyLights::setDelay(unsigned long delay_value) {
  // Set new delay. Must be between DELAY_MIN and DELAY_MAX.
  fstate.delay = constrain(delay_value, DELAY_MIN, DELAY_MAX);
  return fstate.delay;
}

uint8_t FairyLights::setStep(int step_value) {
  // Set new step. Must be between STEP_MIN and STEP_MAX.
  fstate.step = constrain(step_value, STEP_MIN, STEP_MAX);
  return fstate.step;
}

FairyState FairyLights::getState() {
  // Getter for the state
  return fstate;
}
