/*
 main.h - Main runtime for FairyLights.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef main_h
#define main_h

#include <Arduino.h>
#include <functional>
#include "FairyLights.h"

typedef std::function<void (int)> FairyCallback;

void initDefaults();

void OTAinit();

void setup_wifi();

int getIntFromBytePayload(byte* payload, unsigned int length);
void genericSerialLog(const char* description, const int new_value);
void setMode(FairyLights &fairyLight, int new_value);
void setBrightness(FairyLights &fairyLight, int new_value);
void setFade(FairyLights &fairyLight, byte* payload, unsigned int length);
void setGeneric(const FairyCallback &fairyCallback, const char* topic, const char* description, int new_value);
void callback(char* topic, byte* payload, unsigned int length);

void sendStatus();

void reconnect();

#endif
