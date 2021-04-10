/*
 FairyLights.h - Simple controller for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 The PWM of one or more pins is driven according to user-selected modes.
 Methods:

 FairyLights(pin) - Constructor. Specify pin to drive
 pattern(value) - Sets the mode for the fairylights. Accepts: integers 0 to 5.
 setMax(value) - Sets the maximum brightness for the fairylights. Accepts: integers 0 to 100.
 TBC
*/

#ifndef FairyLights_h
#define FairyLights_h

#include "cie1931.h"
#include <Arduino.h>
#include <functional>
#include "Pattern.h"
#include "NullPattern.h"
#include "GoalPattern.h"
#include "FadeBiPattern.h"
#include "FadeUpPattern.h"
#include "FadeDownPattern.h"
#include "BlinkPattern.h"
#include "FadeRandPattern.h"
#include "BumpPattern.h"
#include "SparklePattern.h"
#include "FairyCommon.h"

#define ON true
#define OFF false



#define STD_BRI 10
#define STD_MIN 0
#define STD_MAX 100
#define STD_STEP 1
#define STD_DELAY 100

#define DELAY_MIN 1
#define DELAY_MAX 5000
#define STEP_MIN 1
#define STEP_MAX 20
#define PWM_MIN 0
#define PWM_MAX 1024
#define INPUT_MIN 0
#define INPUT_MAX 100


class FairyLights
{
public:
 FairyLights();   // Empty constructor - apparently that's the Arduino way ...
 void setup(int pin); //Constructor. attach pin to drive, set standard brightness
 void setup(int pin, int brightness, int min, int max, int step, unsigned long delay); //Constructor. attach pin to drive, set custom brightness
 void loop(); //loop function that needs to be called regularly to update fairylights. Calls handler according to selected interval
 void handle(); //actual untimed handler
 uint8_t setGoal(int brightness, bool fade = false, bool savePrev = true, uint8_t custom_delay = 0); //set brightness goal
 uint8_t revertGoal(bool fade = false); // Go back to previously saved brightness
 Mode setMode(Mode mode_value);  //set mode
 uint8_t setMax(int max_value);
 uint8_t setMin(int min_value);
 uint8_t setDelay(unsigned long delay_value);
 uint8_t setStep(int step_value);
 uint8_t bump(int bump_value);
 FairyState getState();
private:
 unsigned long previousMillis = 0;
 unsigned long currentMillis;
 unsigned long evaluated_delay = 0;
 void initBrightness(int brightness); //set values for initial brightness setting
 void applyBrightness(); //apply brightness setting to pin
 void cycleCallback();
 void setupPattern(Mode mode_value);
 /*
  * Currently, every pattern must be declared and initialised individually.
  * Then, a reference to the pattern must be added to the patterns array.
  * The reference MUST NOT be the first or the last element in the array!
  * In addition, the Mode enum (defined in FairyCommon.h) must be adapted
  * and take the order of the patterns array into account.
  */
 NullPattern patt_null;
 GoalPattern patt_goal;
 FadeBiPattern patt_fadebi;
 FadeUpPattern patt_fadeup;
 FadeDownPattern patt_fadedown;
 FadeRandPattern patt_faderand;
 BlinkPattern patt_blink;
 BumpPattern patt_bump;
 SparklePattern patt_sparkle;
 Pattern *patterns[9] = {&patt_null, &patt_fadebi, &patt_fadeup, &patt_fadedown, &patt_faderand, &patt_blink, &patt_bump, &patt_sparkle, &patt_null};
 uint8_t pinNumber;
 FairyState fstate;
 int8_t prevBri = -1;
};

#endif
