//flag for saving data (WiFiManager)
bool shouldSaveConfig = false;

// Callback notifying that config needs to be saved (WiFIManager)
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void WiFiManagerSetup() {
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

          strcpy(light_name, json["light_name"]);
          strcpy(light_nick, json["light_nick"]);
          // strcpy(light_secret, json["light_secret"]);

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
  WiFiManagerParameter custom_light_name("lightName", "Light identifier", light_name, 40);
  WiFiManagerParameter custom_light_nick("lightNick", "Light display name", light_nick, 40);
  //WiFiManagerParameter custom_light_secret("lightSecret", "Light group", light_secret, 40);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  wifiManager.addParameter(&custom_light_name);
  wifiManager.addParameter(&custom_light_nick);
  //wifiManager.addParameter(&custom_light_secret);

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
  strcpy(light_name, custom_light_name.getValue());
  strcpy(light_nick, custom_light_nick.getValue());
  // strcpy(light_secret, custom_light_secret.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["light_name"] = light_name;
    json["light_nick"] = light_nick;
    // json["light_secret"] = light_secret;

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
