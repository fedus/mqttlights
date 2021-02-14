/*
 NullPattern.cpp - Null pattern class for fairylights, such as LED strings.
 (c) 2019 Federico Gentile
*/

#include <Arduino.h>
#include "NullPattern.h"
#include "Pattern.h"
#include "FairyCommon.h"

NullPattern::NullPattern(): Pattern() {
  // Empty constructor
}

FairyState NullPattern::getNext(FairyState &fstate) {
  return fstate;
}
