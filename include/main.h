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
#include "GeneralSettings.h"
#include "FairyContainer.h"

typedef std::function<void (int)> FairyCallback;

void OTAinit();

void setup_wifi();
void activate_fairylights();

int getIntFromBytePayload(byte* payload, unsigned int length);
void genericSerialLog(const char* description, const int new_value);
void setMode(FairyMQTTBinder &fairyBinder, int new_value);
void setBrightness(FairyMQTTBinder &fairyBinder, int new_value);
void setFade(FairyMQTTBinder &fairyBinder, byte* payload, unsigned int length);
void setGeneric(const FairyCallback &fairyCallback, FairyMQTTBinder &fairyBinder, const char* topic, const char* description, int new_value);
void callback(char* topic, byte* payload, unsigned int length);

void extractTopicParts(const char* topic);

void sendStatus(FairyMQTTBinder &fairyBinder);

void reconnect();

#endif
