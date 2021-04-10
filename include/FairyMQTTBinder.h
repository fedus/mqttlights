/*
 FairyMQTTBinder.h - FairyLight-to-MQTT binder / container
 (c) 2019-2021 Federico Gentile
*/

#ifndef FairyMQTTBinder_h
#define FairyMQTTBinder_h

#include "FairyLights.h"

class FairyMQTTBinder
{
public:
 FairyMQTTBinder();  //Constructor
 bool setup(int pin, const char* name, const char* nick, const char* realm = "public"); //associate with given FairyLight.
 void associate(FairyLights &fairyLight);
 bool isActive();
 const char* getNick();
 const char* getName();
 const char* getRealm();
 bool isName(const char* otherName);
 bool isNick(const char* otherNick);
 bool isRealm(const char* otherRealm);
 FairyLights& getFairyLight();
 void loop();
private:
 FairyLights *fairyLight;
 bool active = false;
 char name[40];
 char nick[40];
 char realm[40];
};

#endif
