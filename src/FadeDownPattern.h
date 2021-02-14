/*
 FadeUpPattern.h - Fade Down pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef FadeDownPattern_h
#define FadeDownPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class FadeDownPattern: public Pattern
{
public:
 FadeDownPattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
};
 
#endif
