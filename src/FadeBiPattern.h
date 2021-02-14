/*
 FadeBiPattern.h - In/Out fading pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef FadeBiPattern_h
#define FadeBiPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class FadeBiPattern: public Pattern
{
public:
 FadeBiPattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
};

#endif
