/*
 FairyWifiManager.h - WifiManager related declarations for FairyLights.
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef FairyWifiManager_h
#define FairyWifiManager_h

#include <Arduino.h>

#if defined(ESP32)
#include "SPIFFS.h"
#else
#include "FS.h"
#endif

#include <WiFiManager.h>
#include "GeneralSettings.h"
#include <ArduinoJson.h>

void saveConfigCallback();

void WiFiManagerSetup(char (*lightConfig)[4][40]);

#endif
