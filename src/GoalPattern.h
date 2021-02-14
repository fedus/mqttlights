/*
 GoalPattern.h - Goal driven pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef GoalPattern_h
#define GoalPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class GoalPattern: public Pattern
{
public:
 GoalPattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
 void setCustomDelay(unsigned long custom_delay);
};

#endif
