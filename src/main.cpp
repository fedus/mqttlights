#include <Arduino.h>
#include "main.h"

#if defined(ESP32)
#include <WiFi.h>
#include <analogWrite.h>
#include "Tree.h"
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Bedroom.h"
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

const char* ota_hostname = OTA_HOSTNAME;
const char* ota_passwd = OTA_PASSWD;

char light_name[40] = LIGHT_NAME;
char light_nick[40] = LIGHT_NICK;

char sub_topic[100];
char pub_topic[100];
char will_topic[100];
const char* discovery_topic = "lux/discovery";

const char will_message[] = "offline";
const char online_keyword[] = "online";

// PIN Settings
const int fairyPin = FAIRY_PIN;     // 5 on Nodemcu, 27 on feather
const int builtinPin = BUILTIN_PIN; // 2 on Nodemcu, 13 on feather
const int builtinOn = BUILTIN_ON;
const int builtinOff = BUILTIN_OFF;

// FairyLight class
FairyLights fairy;

bool ota_blink = false;

int mqtt_delay = 15;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

char *fade_parser;
uint8_t fade_parser_index = 0;
unsigned long fade_params[] = {0, 0};
char topicBuffer[100];


void initDefaults() {
  sprintf(sub_topic, "%s%s%s", "lux/lights/", light_name, "/control/");
  sprintf(pub_topic, "%s%s%s", "lux/lights/", light_name, "/status/");
  sprintf(will_topic, "%s%s", pub_topic, "connection");
}

void OTAinit() {
  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(ota_passwd);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    fairy.setGoal(INPUT_MAX, false, false);
    fairy.handle();
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
    fairy.setGoal(_ota_bright, false, false);
    fairy.handle();
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    fairy.setMode(BLINK);
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

void sendStatus() {
    Serial.print("[ Sending status ]");

    FairyState fstate = fairy.getState();

    client.publish(will_topic, online_keyword, true);
    sprintf(topicBuffer, "%s%s", pub_topic, "mode");
    client.publish(topicBuffer, String(fstate.mode).c_str());

    delay(mqtt_delay);

    if (fstate.mode == STATIC) {
      if (fstate.curr == fstate.goal) {
        sprintf(topicBuffer, "%s%s", pub_topic, "brightness");
        client.publish(topicBuffer, String(fstate.curr).c_str());
      }
      else {
        sprintf(topicBuffer, "%s%s", pub_topic, "fade");
        client.publish(topicBuffer, String(fstate.goal).c_str());
      }
    }
    delay(mqtt_delay);
    sprintf(topicBuffer, "%s%s", pub_topic, "step");
    client.publish(topicBuffer, String(fstate.step).c_str());
    delay(mqtt_delay);
    sprintf(topicBuffer, "%s%s", pub_topic, "min");
    client.publish(topicBuffer, String(fstate.min).c_str());
    delay(mqtt_delay);
    sprintf(topicBuffer, "%s%s", pub_topic, "max");
    client.publish(topicBuffer, String(fstate.max).c_str());
    delay(mqtt_delay);
    sprintf(topicBuffer, "%s%s", pub_topic, "delay");
    client.publish(topicBuffer, String(fstate.delay).c_str());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "mqtt-fairylight-"+String(light_name);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_passwd, will_topic, 0, 1, will_message)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(will_topic, online_keyword, true);
      // ... and resubscribe
      sprintf(topicBuffer, "%s%s", sub_topic, "#");
      client.subscribe(topicBuffer);
      client.subscribe(discovery_topic);
      fairy.revertGoal(false);
      sendStatus();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setMode(FairyLights &fairyLight, int new_value) {
    Serial.print("[ Setting mode ");
    Serial.print(new_value);
    Serial.println(" ]");

    fairyLight.setMode((Mode)new_value);

    sprintf(topicBuffer, "%s%s", pub_topic, "mode");
    client.publish(topicBuffer, String(new_value).c_str());

    if (new_value == STATIC) {
      FairyState fstate = fairyLight.getState();

      if (fstate.curr == fstate.goal) {
        sprintf(topicBuffer, "%s%s", pub_topic, "brightness");
        client.publish(topicBuffer, String(fstate.curr).c_str());
      }
      else {
        sprintf(topicBuffer, "%s%s", pub_topic, "fade");
        client.publish(topicBuffer, String(fstate.goal).c_str());
      }
    }
}

void setBrightness(FairyLights &fairyLight, int new_value) {
    Serial.print("[ Setting brightness to ");
    Serial.print(new_value);
    Serial.println(" ]");
    fairyLight.setGoal(new_value, false, true);

    if(fairyLight.getState().mode != STATIC) {
      fairyLight.setMode(STATIC);
      sprintf(topicBuffer, "%s%s", pub_topic, "mode");
      client.publish(topicBuffer, String(STATIC).c_str());
      delay(mqtt_delay);
    }

    sprintf(topicBuffer, "%s%s", pub_topic, "brightness");
    client.publish(topicBuffer, String(new_value).c_str());
    sprintf(topicBuffer, "%s%s", pub_topic, "debug/cie");
    client.publish(topicBuffer, String(cie[new_value]).c_str());
}

void setFade(FairyLights &fairyLight, byte* payload, unsigned int length) {
    // For fading, we need to set to STATIC before setting the new goal (due to goal's setup)
    if(fairyLight.getState().mode != STATIC) {
      fairyLight.setMode(STATIC);
      sprintf(topicBuffer, "%s%s", pub_topic, "mode");
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

    //int new_value = atoi((char *)payload);
    Serial.print("[ Fading brightness to ");
    Serial.print(fade_params[0]);
    Serial.print(" with fade delay ");
    Serial.print(fade_params[1]);
    Serial.println(" ]");
    fairyLight.setGoal(fade_params[0], true, true, fade_params[1]);

    sprintf(topicBuffer, "%s%s", pub_topic, "fade");
    client.publish(topicBuffer, String(fade_params[0]).c_str());
}

void setGeneric(const FairyCallback &fairyCallback, const char* topic, const char* description, int new_value) {
    Serial.print("[ Setting ");
    Serial.print(description);
    Serial.print(" to ");
    Serial.print(new_value);
    Serial.println(" ]");
    fairyCallback(new_value);

    sprintf(topicBuffer, "%s%s", pub_topic, topic);
    client.publish(topicBuffer, String(new_value).c_str());
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

  // Answer to discovery request
  if(strcmp(topic, discovery_topic) == 0) {
    Serial.print("[ Answering to discovery request ]");

    sprintf(topicBuffer, "%s%s%s", discovery_topic, "/", light_name);
    client.publish(topicBuffer, light_nick);
  }

  // Set brightness instantaneously
  sprintf(topicBuffer, "%s%s", sub_topic, "brightness");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int new_value = atoi((char *)payload);

    setBrightness(fairy, new_value);
  }

  // Set brightness by fading
  sprintf(topicBuffer, "%s%s", sub_topic, "fade");
  if(strcmp(topic, topicBuffer) == 0) {
    setFade(fairy, payload, length);
  }

  // Set loop delay
  sprintf(topicBuffer, "%s%s", sub_topic, "delay");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int new_value = atoi((char *)payload);

    setGeneric(
      std::bind(&FairyLights::setDelay, std::ref(fairy), std::placeholders::_1),
      "delay",
      "loop delay",
      new_value
    );
  }

  // Set min brightness
  sprintf(topicBuffer, "%s%s", sub_topic, "min");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int new_value = atoi((char *)payload);

    setGeneric(
      std::bind(&FairyLights::setMin, std::ref(fairy), std::placeholders::_1),
      "min",
      "MIN brightness",
      new_value
    );
  }

  // Set max brightness
  sprintf(topicBuffer, "%s%s", sub_topic, "max");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int new_value = atoi((char *)payload);

    setGeneric(
      std::bind(&FairyLights::setMax, std::ref(fairy), std::placeholders::_1),
      "max",
      "MAX brightness",
      new_value
    );
  }

  // Set brightness step
  sprintf(topicBuffer, "%s%s", sub_topic, "step");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int new_value = _max(1, atoi((char *)payload));

    setGeneric(
      std::bind(&FairyLights::setStep, std::ref(fairy), std::placeholders::_1),
      "step",
      "step",
      new_value
    );
  }

  sprintf(topicBuffer, "%s%s", sub_topic, "bump");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int amount = _max(1, atoi((char *)payload));

    setGeneric(
      std::bind(&FairyLights::bump, std::ref(fairy), std::placeholders::_1),
      "bump",
      "bump",
      amount
    );
  }


  sprintf(topicBuffer, "%s%s", sub_topic, "get_status");
  if(strcmp(topic, topicBuffer) == 0) {
    sendStatus();
  }

  sprintf(topicBuffer, "%s%s", sub_topic, "mode");
  if(strcmp(topic, topicBuffer) == 0) {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    int new_mode = atoi((char *)payload);

    setMode(fairy, new_mode);
  }

  // Turn built-in LED off
  digitalWrite(builtinPin, builtinOff);
}

void setup() {
  initDefaults();
  randomSeed(analogRead(0));  // Because needed by some parts like the FairyLight lib
  Serial.begin(115200);
  Serial.println("SETUP");
  pinMode(builtinPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(builtinPin, builtinOff);
  fairy.setup(fairyPin);
  fairy.setGoal(5, false, false);
  fairy.handle();
  //setup_wifi();
  WiFiManagerSetup(light_name, light_nick);
  fairy.setGoal(0, false, false);
  fairy.handle();
  OTAinit();
  // Set brightness to default value after signalling wifi setup
  fairy.setGoal(STD_BRI, false, false);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    digitalWrite(builtinPin, builtinOn);
    fairy.setGoal(0, false, true);
    fairy.handle();
    delay(50);
    fairy.setGoal(10, false, false);
    fairy.handle();
    delay(50);
    fairy.setGoal(0, false, false);
    fairy.handle();
    reconnect();
    digitalWrite(builtinPin, builtinOff);
  }
  client.loop();
  fairy.loop();
}