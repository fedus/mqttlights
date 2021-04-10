/*
 GeneralSettings.h - General settings file
 (c) 2019 Federico Gentile

 Methods:

 TODO
*/

#ifndef GeneralSettings_h
#define GeneralSettings_h

#define FAIRY_AMOUNT 3

#define DEFAULT_DEVICE_NAME "fairylight"

#define MQTT_ROOT_TOPIC "lux"
#define MQTT_DISCOVERY_TOPIC "discovery"
#define MQTT_DEVICES_TOPIC "topic"
#define MQTT_LIGHTS_TOPIC "lights"
#define MQTT_CONTROL_TOPIC "control"
#define MQTT_STATUS_TOPIC "status"
#define MQTT_WILL_MESSAGE "offline"
#define MQTT_ONLINE_KEYWORD "online"
#define MQTT_CONNECTION_TOPIC "connection"

#define MAIN_TOPIC_POS 0
#define DEVICES_OR_DISCOVERY_TOPIC_POS 1
#define DEVICE_TOPIC_POS 2
#define DISCOVERY_REALM_TOPIC_POS 2
#define LIGHTS_TOPIC_POS 3
#define DISCOVERED_DEVICE_TOPIC 3
#define LIGHT_TOPIC_POS 4
#define DISCOVERED_LIGHT__TOPIC 4
#define CONTROL_TOPIC_POS 5
#define COMMAND_TOPIC_POS 6


#endif
