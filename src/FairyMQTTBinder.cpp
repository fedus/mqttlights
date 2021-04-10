/*
 FairyMQTTBinder.cpp - FairyLight-to-MQTT binder / container
 (c) 2019-2021 Federico Gentile
*/

#include "FairyMQTTBinder.h"

FairyMQTTBinder::FairyMQTTBinder() {
  // Empty constructor
}

bool FairyMQTTBinder::setup(int pin, const char* name, const char* nick, const char* realm /* = "public" */) {
  if (this->fairyLight != nullptr) {
    return false;
  }

  this->fairyLight->setup(pin);

  active = true;
  snprintf(this->name, sizeof(this->name), "%", name);
  snprintf(this->nick, sizeof(this->nick), "%s", nick);
  snprintf(this->realm, sizeof(this->realm), "%s", realm);

  return true;
}

void FairyMQTTBinder::associate(FairyLights &fairyLight) {
  this->fairyLight = &fairyLight;
}

bool FairyMQTTBinder::isActive() {
  return active;
}

const char* FairyMQTTBinder::getNick() {
  return nick;
}

const char* FairyMQTTBinder::getName() {
  return name;
}

const char* FairyMQTTBinder::getRealm() {
  return realm;
}

bool FairyMQTTBinder::isNick(const char* otherNick) {
  return strcmp(nick, otherNick);
}

bool FairyMQTTBinder::isName(const char* otherName) {
  return strcmp(name, otherName);
}

bool FairyMQTTBinder::isRealm(const char* otherRealm) {
  return strcmp(realm, otherRealm);
}

FairyLights& FairyMQTTBinder::getFairyLight() {
  return *fairyLight;
}

void FairyMQTTBinder::loop() {
  getFairyLight().loop();
}
