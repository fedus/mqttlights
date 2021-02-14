/*
 FadeRandPattern.h - Random hi/lo fading pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef FadeRandPattern_h
#define FadeRandPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class FadeRandPattern: public Pattern
{
public:
 FadeRandPattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
private:
 bool highGoal = true;
};
 
#endif
