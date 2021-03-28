/*
 FadeUpPattern.h - Fade Up pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef FadeUpPattern_h
#define FadeUpPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class FadeUpPattern: public Pattern
{
public:
 FadeUpPattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
};
 
#endif
