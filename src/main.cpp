#include <Arduino.h>
#include "main.h"

#if defined(ESP32)
#include <WiFi.h>
#include <analogWrite.h>
#include "E32.h"
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "E8266.h"
#endif

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <FairyWifiManager.h>
#include "cie1931.h"
#include "auth.h"
#include "FairyLights.h"


// Update these with values suitable for your network.

char wifi_ssid[40] = WIFI_SSID;
char wifi_passwd[40] = WIFI_PASSWD;

char mqtt_server[40] = MQTT_HOST;
char mqtt_user[40] = MQTT_USER;
char mqtt_passwd[40] = MQTT_PASSWD;

const char* mqttRootTopic = MQTT_ROOT_TOPIC;
const char* mqttDiscoveryTopic = MQTT_DISCOVERY_TOPIC;

const char* ota_hostname = OTA_HOSTNAME;
const char* ota_passwd = OTA_PASSWD;

char device_name[] = DEFAULT_DEVICE_NAME; // TODO: get from config

const char* will_message = MQTT_WILL_MESSAGE;
const char* online_keyword = MQTT_ONLINE_KEYWORD;

// PIN Settings
//const int fairyPin;     // 5 on Nodemcu, 27 on feather
const int builtinPin = BUILTIN_PIN; // 2 on Nodemcu, 13 on feather
const int builtinOn = BUILTIN_ON;
const int builtinOff = BUILTIN_OFF;

// FairyLight class
FairyContainer fairyContainer;

bool ota_blink = false;

int mqtt_delay = 15;

WiFiClient espClient;
PubSubClient client(espClient);

char *fade_parser;
uint8_t fade_parser_index = 0;
unsigned long fade_params[] = {0, 0};
char topicBuffer[100];
char topicParts[10][40];

char lightConfig[FAIRY_AMOUNT][4][40];

void OTAinit() {
  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(ota_passwd);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    fairyContainer.getDefaultFairyLight().setGoal(INPUT_MAX, false, false);
    fairyContainer.getDefaultFairyLight().handle();
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int _ota_bright;
    if (ota_blink == true) {
      _ota_bright = INPUT_MIN;
      ota_blink = false;
    }
    else {
      _ota_bright = INPUT_MAX;
      ota_blink = true;
    }
    fairyContainer.getDefaultFairyLight().setGoal(_ota_bright, false, false);
    fairyContainer.getDefaultFairyLight().handle();
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    fairyContainer.getDefaultFairyLight().setMode(BLINK);
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_passwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendStatus(FairyMQTTBinder &fairyBinder) {
    Serial.println("[ Sending status ]");

    FairyState fstate = fairyBinder.getFairyLight().getState();

    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getWillTopic());
    client.publish(topicBuffer, online_keyword, true);
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "mode"));
    client.publish(topicBuffer, String(fstate.mode).c_str());

    delay(mqtt_delay);

    if (fstate.mode == STATIC) {
      if (fstate.curr == fstate.goal) {
        snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "brightness"));
        client.publish(topicBuffer, String(fstate.curr).c_str());
      }
      else {
        snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "fade"));
        client.publish(topicBuffer, String(fstate.goal).c_str());
      }
    }
    delay(mqtt_delay);
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "step"));
    client.publish(topicBuffer, String(fstate.step).c_str());
    delay(mqtt_delay);
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "min"));
    client.publish(topicBuffer, String(fstate.min).c_str());
    delay(mqtt_delay);
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "max"));
    client.publish(topicBuffer, String(fstate.max).c_str());
    delay(mqtt_delay);
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "delay"));
    client.publish(topicBuffer, String(fstate.delay).c_str());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "mqtt-fairylight-"+String(device_name);
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getWillTopic());
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_passwd, topicBuffer, 0, 1, will_message)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topicBuffer, online_keyword, true);
      // ... and resubscribe
      fairyContainer.doForAllActiveFairyBinders([](FairyMQTTBinder fairyBinder) -> void {
        snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getSubTopic(fairyBinder));
        client.subscribe(topicBuffer);
      });
      snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getDiscoveryTopic());
      client.subscribe(topicBuffer);
      fairyContainer.getDefaultFairyLight().revertGoal(false); // TODO: do for all
      fairyContainer.doForAllActiveFairyBinders([](FairyMQTTBinder fairyBinder) -> void { sendStatus(fairyBinder); }); // TODO: gives away all realms
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int getIntFromBytePayload(byte* payload, unsigned int length) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    return atoi((char *)payload);
}

void genericSerialLog(const char* description, const int new_value) {
    Serial.print("[ Setting ");
    Serial.print(description);
    Serial.print(" to ");
    Serial.print(new_value);
    Serial.println(" ]");
}

void setMode(FairyMQTTBinder &fairyBinder, int new_value) {
    FairyLights &fairyLight = fairyBinder.getFairyLight();

    genericSerialLog("mode", new_value);

    fairyLight.setMode((Mode)new_value);

    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "mode"));
    client.publish(topicBuffer, String(new_value).c_str());

    if (new_value == STATIC) {
      FairyState fstate = fairyLight.getState();

      if (fstate.curr == fstate.goal) {
        snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "brightness"));
        client.publish(topicBuffer, String(fstate.curr).c_str());
      }
      else {
        snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "fade"));
        client.publish(topicBuffer, String(fstate.goal).c_str());
      }
    }
}

void setBrightness(FairyMQTTBinder &fairyBinder, int new_value) {
    FairyLights &fairyLight = fairyBinder.getFairyLight();

    genericSerialLog("brightness", new_value);
    fairyLight.setGoal(new_value, false, true);

    if(fairyLight.getState().mode != STATIC) {
      fairyLight.setMode(STATIC);
      snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "mode"));
      client.publish(topicBuffer, String(STATIC).c_str());
      delay(mqtt_delay);
    }

    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "brightness"));
    client.publish(topicBuffer, String(new_value).c_str());
    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "debug/cie"));
    client.publish(topicBuffer, String(cie[new_value]).c_str());
}

void setFade(FairyMQTTBinder &fairyBinder, byte* payload, unsigned int length) {
    FairyLights &fairyLight = fairyBinder.getFairyLight();

    // For fading, we need to set to STATIC before setting the new goal (due to goal's setup)
    if(fairyLight.getState().mode != STATIC) {
      fairyLight.setMode(STATIC);
      snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "mode"));
      client.publish(topicBuffer, String(STATIC).c_str());
      delay(mqtt_delay);
    }

    payload[length] = '\0'; // Make payload a string by NULL terminating it.

    char parsedPayload[length+1];
    for (int i=0;i<length;i++) {
      parsedPayload[i] = (char)payload[i];
    }
    parsedPayload[length] = NULL;

    fade_parser_index = 0;
    fade_params[0] = 0;
    fade_params[1] = 0;
    fade_parser = strtok(parsedPayload, ",");

    while(fade_parser != NULL) {
      fade_params[fade_parser_index++] = atoi(fade_parser);
      fade_parser=strtok(NULL, ",");
    }

    Serial.print("[ Fading brightness to ");
    Serial.print(fade_params[0]);
    Serial.print(" with fade delay ");
    Serial.print(fade_params[1]);
    Serial.println(" ]");
    fairyLight.setGoal(fade_params[0], true, true, fade_params[1]);

    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, "fade"));
    client.publish(topicBuffer, String(fade_params[0]).c_str());
}

void setGeneric(const FairyCallback &fairyCallback, FairyMQTTBinder &fairyBinder, const char* topic, const char* description, int new_value) {
    genericSerialLog(description, new_value);
    fairyCallback(new_value);

    snprintf(topicBuffer, sizeof(topicBuffer), "%s", fairyContainer.getPubTopic(fairyBinder, topic));
    client.publish(topicBuffer, String(new_value).c_str());
}

void extractTopicParts(const char* topic) {
  for (int i = 0; i < 10; i++) {
    topicParts[i][0] = 0;
  }

  char topicCopy[100];
  snprintf(topicCopy, sizeof(topicCopy), "%s", topic);

  int parser_index = 0;

  char *parser = strtok(topicCopy, "/");

  while(parser != NULL) {
    snprintf(topicParts[parser_index++], sizeof(topicParts[parser_index]), "%s", parser);
    parser=strtok(NULL, "/");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Use built-in LED to show message processing
  digitalWrite(builtinPin, builtinOn);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  extractTopicParts(topic);

  // Answer to discovery request
  if(
    strcmp(topicParts[DEVICES_OR_DISCOVERY_TOPIC_POS], MQTT_DISCOVERY_TOPIC) == 0
    && strcmp(topicParts[DISCOVERED_DEVICE_TOPIC], "") == 0 // Only respond to discovery request, not answers
  ) {
    Serial.print("[ Answering to discovery request for realm ");
    Serial.print(topicParts[DISCOVERY_REALM_TOPIC_POS]);
    Serial.println("]");

    fairyContainer.doForAllActiveFairyBindersInRealm([&](FairyMQTTBinder &fairyBinder) -> void {
      snprintf(
        topicBuffer,
        sizeof(topicBuffer),
        "%s/%s/%s/%s/%s",
        mqttRootTopic,
        mqttDiscoveryTopic,
        topicParts[DISCOVERY_REALM_TOPIC_POS],
        fairyContainer.getDeviceName(),
        fairyBinder.getName()
      );
      client.publish(topicBuffer, fairyBinder.getNick());
    }, topicParts[DISCOVERY_REALM_TOPIC_POS]);

  } else if (fairyContainer.hasFairyBinder(topicParts[LIGHT_TOPIC_POS])) {

    FairyMQTTBinder &fairyBinder = fairyContainer.getFairyBinder(topicParts[LIGHT_TOPIC_POS]);
    FairyLights &fairyLight = fairyBinder.getFairyLight();

    // Set brightness by fading
    if(strcmp(topicParts[COMMAND_TOPIC_POS], "fade") == 0) {
      setFade(fairyBinder, payload, length);
    }

    // Set brightness instantaneously
    if(strcmp(topicParts[COMMAND_TOPIC_POS], "brightness") == 0) {
      setBrightness(fairyBinder, getIntFromBytePayload(payload, length));
    }

    // Set loop delay
    if(strcmp(topicParts[COMMAND_TOPIC_POS], "delay") == 0) {
      setGeneric(
        std::bind(&FairyLights::setDelay, std::ref(fairyLight), std::placeholders::_1),
        fairyBinder,
        "delay",
        "loop delay",
        getIntFromBytePayload(payload, length)
      );
    }

    // Set min brightness
    if(strcmp(topicParts[COMMAND_TOPIC_POS], "min") == 0) {
      setGeneric(
        std::bind(&FairyLights::setMin, std::ref(fairyLight), std::placeholders::_1),
        fairyBinder,
        "min",
        "MIN brightness",
        getIntFromBytePayload(payload, length)
      );
    }

    // Set max brightness
    if(strcmp(topicParts[COMMAND_TOPIC_POS], "max") == 0) {
      setGeneric(
        std::bind(&FairyLights::setMax, std::ref(fairyLight), std::placeholders::_1),
        fairyBinder,
        "max",
        "MAX brightness",
        getIntFromBytePayload(payload, length)
      );
    }

    // Set brightness step
    if(strcmp(topicParts[COMMAND_TOPIC_POS], "step") == 0) {
      int new_value = _max(1, getIntFromBytePayload(payload, length));

      setGeneric(
        std::bind(&FairyLights::setStep, std::ref(fairyLight), std::placeholders::_1),
        fairyBinder,
        "step",
        "step",
        new_value
      );
    }

    if(strcmp(topicParts[COMMAND_TOPIC_POS], "bump") == 0) {
      int amount = _max(1, getIntFromBytePayload(payload, length));

      setGeneric(
        std::bind(&FairyLights::bump, std::ref(fairyLight), std::placeholders::_1),
        fairyBinder,
        "bump",
        "bump",
        amount
      );
    }


    if(strcmp(topicParts[COMMAND_TOPIC_POS], "get_status") == 0) {
      sendStatus(fairyBinder);
    }

    if(strcmp(topicParts[COMMAND_TOPIC_POS], "mode") == 0) {
      setMode(fairyBinder, getIntFromBytePayload(payload, length));
    }
  }

  // Turn built-in LED off
  digitalWrite(builtinPin, builtinOff);
}

void activate_fairylights() {
  for (int i = 0; i < FAIRY_AMOUNT; i++) {
    if (
      strcmp(lightConfig[i][0], "") != 0
      && strcmp(lightConfig[i][1], "") != 0
      && strcmp(lightConfig[i][2], "") != 0
      && strcmp(lightConfig[i][3], "") != 0
    ) {
      int pin = atoi((char *)lightConfig[i][3]);

      Serial.print("[Activating new fairylight with name ");
      Serial.print(lightConfig[i][0]);
      Serial.print(" and nick ");
      Serial.print(lightConfig[i][1]);
      Serial.print(" in realm ");
      Serial.print(lightConfig[i][2]);
      Serial.print(" on pin ");
      Serial.print(pin);
      Serial.println("]");

      fairyContainer.activateNew(
        pin,
        lightConfig[i][0],
        lightConfig[i][1],
        lightConfig[i][2]
      );
    }
  }
}

void setup() {
  randomSeed(analogRead(0));  // Because needed by some parts like the FairyLight lib
  Serial.begin(115200);
  Serial.println("SETUP");
  pinMode(builtinPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(builtinPin, builtinOn);
  fairyContainer.setup(device_name);
  WiFiManagerSetup(lightConfig);
  activate_fairylights();
  OTAinit();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(builtinPin, builtinOff);
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    digitalWrite(builtinPin, builtinOn);
    reconnect();
    digitalWrite(builtinPin, builtinOff);
  }
  client.loop();
  fairyContainer.loop();
}