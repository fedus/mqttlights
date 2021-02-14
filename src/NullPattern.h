/*
 NullPattern.h - Null pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef NullPattern_h
#define NullPattern_h

#include <Arduino.h>
#include "Pattern.h"
#include "FairyCommon.h"

class NullPattern: public Pattern
{
public:
 NullPattern();  //Constructor
 FairyState getNext(FairyState &fstate); //calculate next value according to pattern.
};

#endif
