/*
 SparklePattern.h - Sparkle pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef SparklePattern_h
#define SparklePattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class SparklePattern: public Pattern
{
public:
 SparklePattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
private:
 void toggle(FairyState &fstate); // Toggles on/off
};

#endif
