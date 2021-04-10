/*
 FairyContainer.cpp - FairyLight container for easy managing of multiple instances
 (c) 2019-2021 Federico Gentile
*/

#include "FairyContainer.h"

FairyContainer::FairyContainer() {
  // Empty constructor
}

void FairyContainer::setup(const char* deviceName) {
  for (int i = 0; i < FAIRY_AMOUNT; i++) {
    fairyBinders[i].associate(fairyLights[i]);
  }

  snprintf(this->deviceName, sizeof(this->deviceName), "%s", deviceName);
}

void FairyContainer::doForAllActiveFairyBinders(FairyBinderOp op) {
  for ( FairyMQTTBinder& fairyBinder : fairyBinders )
  {
    if (!fairyBinder.isActive()) continue;
    op(fairyBinder);
  }
}

void FairyContainer::conditionalDoForAllActiveFairyBinders(FairyBinderOp op, FairyBinderOpCondition cond) {
  doForAllActiveFairyBinders([&op, &cond](FairyMQTTBinder &fairyBinder) -> void {
    if (cond(fairyBinder)) op(fairyBinder);
  });
}

void FairyContainer::doForAllActiveFairyBindersInRealm(FairyBinderOp op, const char* realm) {
  conditionalDoForAllActiveFairyBinders(op, [&realm](FairyMQTTBinder &fairyBinder) -> bool {
    return fairyBinder.isRealm(realm);
  });
}

FairyLights& FairyContainer::getFairyLight(const char* name) {
  return getFairyBinder(name).getFairyLight();
}

FairyMQTTBinder& FairyContainer::getFairyBinder(const char* name) {
  for (FairyMQTTBinder& fairyBinder : fairyBinders) {
    if (fairyBinder.isName(name)) return fairyBinder;
  }
}

bool FairyContainer::hasFairyBinder(const char* name) {
  for (FairyMQTTBinder& fairyBinder : fairyBinders) {
    if (fairyBinder.isName(name)) return true;
  }

  return false;
}

FairyMQTTBinder& FairyContainer::getNextFreeFairyBinder() {
  for (FairyMQTTBinder& fairyBinder : fairyBinders) {
    if (!fairyBinder.isActive()) return fairyBinder;
  }
}

bool FairyContainer::hasFreeFairyBinder() {
  for (FairyMQTTBinder& fairyBinder : fairyBinders) {
    if (!fairyBinder.isActive()) return true;
  }

  return false;
}

bool FairyContainer::activateNew(int pin, const char* name, const char* nick, const char* realm /* = "public" */) {
  if (!hasFreeFairyBinder()) return false;

  getNextFreeFairyBinder().setup(pin, name, nick, realm);

  return true;
}

void FairyContainer::loop() {
  doForAllActiveFairyBinders([](FairyMQTTBinder &fairyBinder) -> void { fairyBinder.loop(); });
}

FairyLights& FairyContainer::getDefaultFairyLight() {
  return fairyBinders[0].getFairyLight();
}

const char* FairyContainer::getDeviceName() {
  return deviceName;
}

const char* FairyContainer::getWillTopic() {
  snprintf(
    topicBuffer,
    sizeof(topicBuffer),
    "%s/s%/%s/%s",
    MQTT_ROOT_TOPIC,
    MQTT_DEVICES_TOPIC,
    deviceName,
    MQTT_CONNECTION_TOPIC
  );

  return topicBuffer;
}

const char* FairyContainer::getDiscoveryTopic() {
  snprintf(
    topicBuffer,
    sizeof(topicBuffer),
    "%s/%s/#",
    MQTT_ROOT_TOPIC,
    MQTT_DISCOVERY_TOPIC
  );

  return topicBuffer;
}

const char* FairyContainer::getSubTopic(FairyMQTTBinder &fairyBinder) {
  return buildTopic(fairyBinder.getName(), MQTT_CONTROL_TOPIC, "#");
}

const char* FairyContainer::getPubTopic(FairyMQTTBinder &fairyBinder, const char* command) {
  return buildTopic(fairyBinder.getName(), MQTT_STATUS_TOPIC, command);
}

const char* FairyContainer::buildTopic(const char* lightName, const char* controlOrStatusTopic, const char* suffix) {
  snprintf(
    topicBuffer,
    sizeof(topicBuffer),
    "%s/s%/%s/%s/%s/%s/%s",
    MQTT_ROOT_TOPIC,
    MQTT_DEVICES_TOPIC,
    deviceName,
    MQTT_LIGHTS_TOPIC,
    lightName,
    controlOrStatusTopic,
    suffix
  );

  return topicBuffer;
}