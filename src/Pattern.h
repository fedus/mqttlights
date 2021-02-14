/*
 Pattern.h - Simple pattern base class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef Pattern_h
#define Pattern_h

#include <Arduino.h>
#include <functional>
#include "FairyCommon.h"

//typedef void (*CycleCallback) (void);
typedef std::function<void ()> CycleCallback;

class Pattern
{
public:
 Pattern();  //Constructor.
 virtual FairyState setup(FairyState &fstate); //set up base values for the pattern.
 virtual FairyState getNext(FairyState &fstate) = 0; //calculate next value according to pattern.
 virtual unsigned long getCustomDelay();
 void setCycleCallback(const CycleCallback &callback);
protected:
 uint8_t cycle(FairyState &fstate);
 uint8_t counter = 0;
 unsigned long custom_delay = 0;
 CycleCallback cycle_callback;
};

#endif
