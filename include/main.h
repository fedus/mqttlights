/*
 main.h - Main runtime for FairyLights.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef main_h
#define main_h

#include <Arduino.h>

void initDefaults();

void OTAinit();

void setup_wifi();

void callback(char* topic, byte* payload, unsigned int length);

void sendStatus();

void reconnect();

#endif
