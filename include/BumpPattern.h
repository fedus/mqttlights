/*
 BumpPattern.h - Bump pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef BumpPattern_h
#define BumpPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class BumpPattern: public Pattern
{
public:
 BumpPattern();  //Constructor
 FairyState setup(FairyState &fstate); //set up base values for the pattern.
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
};

#endif
