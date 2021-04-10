/*
 FairyContainer.h - FairyLight container for easy managing of multiple instances
 (c) 2019-2021 Federico Gentile
*/

#ifndef FairyContainer_h
#define FairyContainer_h

#include "GeneralSettings.h"
#include "FairyLights.h"
#include "FairyMQTTBinder.h"

typedef std::function<void (FairyMQTTBinder&)> FairyBinderOp;
typedef std::function<bool (FairyMQTTBinder&)> FairyBinderOpCondition;

class FairyContainer
{
public:
 FairyContainer();  //Constructor
 void setup(const char* deviceName);
 void doForAllActiveFairyBinders(FairyBinderOp op);
 void conditionalDoForAllActiveFairyBinders(FairyBinderOp op, FairyBinderOpCondition cond);
 void doForAllActiveFairyBindersInRealm(FairyBinderOp op, const char* realm);
 bool activateNew(int pin, const char* name, const char* nick);
 void loop();
 FairyLights& getFairyLight(const char* name);
 FairyLights& getDefaultFairyLight();
 FairyMQTTBinder& getFairyBinder(const char* name);
 bool hasFairyBinder(const char* name);
 bool hasFreeFairyBinder();
 const char* getDeviceName();
 const char* getWillTopic();
 const char* getDiscoveryTopic();
 const char* getSubTopic(FairyMQTTBinder &fairyBinder);
 const char* getPubTopic(FairyMQTTBinder &fairyBinder, const char* command);
private:
 const char* buildTopic(const char* lightName, const char* controlOrStatusTopic, const char* suffix);
 FairyMQTTBinder& getNextFreeFairyBinder();
 FairyLights fairyLights[FAIRY_AMOUNT];
 FairyMQTTBinder fairyBinders[FAIRY_AMOUNT];
 char deviceName[50];
 char topicBuffer[100];
};

#endif
