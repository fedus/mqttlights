/*
 FairyWifiManager.cpp - User-friendly(ish) wifi connection handling
 (c) 2019 Federico Gentile
*/

#include <FairyWifiManager.h>

char buffer1[FAIRY_AMOUNT][4][40];
char buffer2[FAIRY_AMOUNT][4][40];

WiFiManagerParameter wifiParams[FAIRY_AMOUNT][4];

//flag for saving data (WiFiManager)
bool shouldSaveConfig = false;

// Callback notifying that config needs to be saved (WiFIManager)
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupBuffers() {
  for (int i = 0; i < FAIRY_AMOUNT; i++) {
    snprintf(buffer1[i][0], sizeof(buffer1[i][0]), "%s%d", "light_name_", i);
    snprintf(buffer1[i][1], sizeof(buffer1[i][1]), "%s%d", "light_nick_", i);
    snprintf(buffer1[i][2], sizeof(buffer1[i][2]), "%s%d", "light_realm_", i);
    snprintf(buffer1[i][3], sizeof(buffer1[i][3]), "%s%d", "light_pin_", i);

    snprintf(buffer2[i][0], sizeof(buffer2[i][0]), "%s %d", "Light identifier", i);
    snprintf(buffer2[i][1], sizeof(buffer2[i][1]), "%s %d", "Light display name", i);
    snprintf(buffer2[i][2], sizeof(buffer2[i][2]), "%s %d", "Light realm", i);
    snprintf(buffer2[i][3], sizeof(buffer2[i][3]), "%s %d", "Light pin", i);
  }
}

void WiFiManagerSetup(char (*lightConfig)[4][40]) {
  setupBuffers();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("Mounting file system...");

  if (SPIFFS.begin()) {
    Serial.println("File system mounted!");
    if (SPIFFS.exists("/fairyConfig.json")) {
      //file exists, reading and loading
      Serial.println("Reading config file ...");
      File configFile = SPIFFS.open("/fairyConfig.json", "r");
      if (configFile) {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nParsed json!");

          for (int i = 0; i < FAIRY_AMOUNT; i++) {
            strcpy(lightConfig[i][0], json[buffer1[i][0]]);
            strcpy(lightConfig[i][1], json[buffer1[i][1]]);
            strcpy(lightConfig[i][2], json[buffer1[i][2]]);
            strcpy(lightConfig[i][3], json[buffer1[i][3]]);
          }

        } else {
          Serial.println("Failed to load json config!");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("Failed to mount file system!");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

  // WiFiManagerParamater's `init` method has been made public for this "hack" to work.
  for (int i = 0; i < FAIRY_AMOUNT; i++) {
    wifiParams[i][0].init(buffer1[i][0], buffer2[i][0], lightConfig[i][0], sizeof(lightConfig[i][0]), "", 1);
    wifiParams[i][1].init(buffer1[i][1], buffer2[i][1], lightConfig[i][1], sizeof(lightConfig[i][1]), "", 1);
    wifiParams[i][2].init(buffer1[i][2], buffer2[i][2], lightConfig[i][2], sizeof(lightConfig[i][2]), "", 1);
    wifiParams[i][3].init(buffer1[i][3], buffer2[i][3], lightConfig[i][3], sizeof(lightConfig[i][3]), "", 1);
  }

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  for (int i = 0; i < FAIRY_AMOUNT; i++) {
    for (WiFiManagerParameter &param : wifiParams[i]) {
      wifiManager.addParameter(&param);
    }
  }

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Fairylight configuration")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Connected to WiFi!");

  //read updated parameters
  for (int i = 0; i < FAIRY_AMOUNT; i++) {
    int innerArraySize = sizeof(wifiParams[FAIRY_AMOUNT])/sizeof(wifiParams[FAIRY_AMOUNT][0]);

    for (int j = 0; j < innerArraySize; j++) {
      snprintf(lightConfig[i][j], sizeof(lightConfig[i][j]), "%s", wifiParams[i][j].getValue());
    }
  }

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    for (int i = 0; i < FAIRY_AMOUNT; i++) {
      json[buffer1[i][0]] = lightConfig[i][0];

      json[buffer1[i][1]] = lightConfig[i][1];

      json[buffer1[i][2]] = lightConfig[i][2];

      json[buffer1[i][3]] = lightConfig[i][3];
    }

    File configFile = SPIFFS.open("/fairyConfig.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("Local ip");
  Serial.println(WiFi.localIP());

}
