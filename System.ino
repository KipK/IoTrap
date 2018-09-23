/// CONFIG \\\

// Callback when entering config mode
void configModeCallback (WiFiManager *myWiFiManager) { 
    flasher.attach(0.2, flash); 
}

//callback notifying the need to save config
void saveConfigCallback () {
  Serial << "Config need to be saved" << endl;
  shouldSaveConfig = true;
}

void saveConfig() {
  if (shouldSaveConfig) {
        Serial <<  F("Saving configuration in '/config.json'") << endl;
        yield();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
//        json["conf_mqtt_server"]    = conf_mqtt_server;
//        json["conf_mqtt_port"]      = conf_mqtt_port;
//        json["conf_mqtt_topic"]     = conf_mqtt_topic;
        json["conf_hostname"]       = conf_hostname;
        #ifdef TELEGRAM
        json["conf_bottoken"]       = conf_bottoken;
        #endif
        SPIFFS.begin();
        File configFile = SPIFFS.open("/config.json", "w");
        
        if (!configFile) {
          
          Serial <<  F("Failed to open '/config.json' file for writing") << endl;
          yield();
          
        }
        json.printTo(Serial);
        Serial << endl;
        json.printTo(configFile);
        configFile.close();
        SPIFFS.end();
        hasConf = true; 
        
    } 
    else {
        Serial << F("Nothing to save") << endl;
    }

    Serial << F("REBOOT NOW!") << endl;
    
    ESP.reset();// This is a bit crude. For some unknown reason webserver can only be started once per boot up 
//    // so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);  
}

void deleteConfig() {
    if (SPIFFS.begin()) {
        SPIFFS.remove("/config.json");
        SPIFFS.end();

        
    }
    else {
      
        Serial <<  F("SPIFFS is probably corrupted, format") << endl;
        yield();
        SPIFFS.format();
    }
    WiFiManager wifiManager;
    wifiManager.resetSettings();
}

void loadConfig() {
    //clean FS, for testing
    //SPIFFS.format();
    Serial <<  F("Mounting SPIFFS File System.") << endl;
    Serial <<  F("It will format if it doesn't exist, hold-on....") << endl;
    yield();     
    if (SPIFFS.begin()) {
        Serial <<  F("Mounted SPIFFS file system") << endl;
        yield();
          if (SPIFFS.exists("/config.json")) {
            File configFile = SPIFFS.open("/config.json", "r");
            
            if (configFile) {
                Serial <<  F("'/config.json' loaded") << endl;
                yield();
                size_t size = configFile.size();
                std::unique_ptr<char[]> buf(new char[size]); // Allocate a buffer to store contents of the file.
                configFile.readBytes(buf.get(), size);
                
                DynamicJsonBuffer jsonBuffer;
                JsonObject& json = jsonBuffer.parseObject(buf.get());
                
                Serial.print(F("config.json => "));
                yield();
                json.printTo(Serial);
                
                if (json.success()) {
                   Serial <<  F("\nParsed '/config.json'") << endl;
                   yield();
                    
//                    strcpy(conf_mqtt_server, json["conf_mqtt_server"]);
//                    strcpy(conf_mqtt_port, json["conf_mqtt_port"]);
//                    strcpy(conf_mqtt_topic, json["conf_mqtt_topic"]);
                    strcpy(conf_hostname, json["conf_hostname"]);
                    #ifdef TELEGRAM
                    if (json["conf_bottoken"]) {
                      strcpy(conf_bottoken, json["conf_bottoken"]);
                      tgrm_ok = true;
                    }
                    else {
                      Serial << "Bot token is missing, please configure it first" << endl;
                    }
                    #endif

                    hasConf = true;
        
                } else {
                    Serial <<  F("Failed to load '/config.json'") << endl;
                    yield();
                    SPIFFS.remove("/config.json");
                    hasConf = false;
                  
                }
                
            } else {
               Serial <<  F("Failed to open 'config.json'") << endl;
               yield();
              
               SPIFFS.remove("/config.json");
               hasConf = false;
            }
            
            yield();
            
          } else {
              Serial <<  F("'/config.json' not found'") << endl;
              yield();
              hasConf = false;
            
          }
      } else {
          Serial <<  F("Failed to mount SPIFFS file System, format,please wait ...") << endl;
          yield();
          SPIFFS.format();
          yield();
          hasConf = false;
    }
    SPIFFS.end();
}

/// END CONFIG \\\

/// WIFI \\

void startWifi() {  
    // Trying to connect to last uesd wifi first.
    WiFi.printDiag(Serial); 
    if (WiFi.SSID()==""){
        Serial << "No Access Point has been set yet. Starting Configuration portal" << endl; 
        hasConf = false;
    }
    else {
        
        
        WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
        unsigned long startedAt = millis();
        Serial << "WIFI: Connection result: " << WiFi.waitForConnectResult() << " Took " << (millis()- startedAt) / 1000 <<  " seconds" << endl;
    }
    yield();

    if (WiFi.status()!=WL_CONNECTED){
        Serial << "WIFI: Failed to connect, finishing setup" << endl;
            
    } else{
        Serial << "WIFI: Connected! IP: " << WiFi.localIP() << endl;
    }
    yield();
    

}

  void startPortal() {
    Serial  <<  F("Starting Captive Portal.") << endl;  
    yield();
    flasher.attach(0.4, flash);
    WiFiManager wifiManager;
    
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setConfigPortalTimeout(360);
    wifiManager.setBreakAfterConfig(true); // Wifimanager close server after quit config

    WiFiManagerParameter custom_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");
    wifiManager.addParameter(&custom_hint);
    
    wifiManager.setCustomHeadElement("<meta charset=\"UTF-8\"><script>window.onload = function(e){ document.getElementsByTagName('div')[1].insertAdjacentHTML('beforebegin', '<br><b>Select Wifi Network:</b>'); document.getElementById('s').insertAdjacentHTML('beforebegin', '<br><b>SSID:</b>'); document.getElementById('p').insertAdjacentHTML('beforebegin', '<br><b>Key:</b>');}</script> ");
   
    WiFiManagerParameter custom_hostname_txt("<br><br><b>Hostname:</b>");   
    wifiManager.addParameter(&custom_hostname_txt);
    
    WiFiManagerParameter custom_hostname("hostname", "", conf_hostname, MAX_HOST);
    wifiManager.addParameter(&custom_hostname);

    #ifdef TELEGRAM
    WiFiManagerParameter custom_bot_id_txt("<br><br><b>Telegram Token:</b>");   
    wifiManager.addParameter(&custom_bot_id_txt);
    
    WiFiManagerParameter custom_bot_id("botid", "Bot Token", conf_bottoken, 50);
    wifiManager.addParameter(&custom_bot_id);
    #endif 
    
//    WiFiManagerParameter custom_mqtt_server_txt("<br><br><b>MQTT: Server adress: (IP or hostname)</b>");
//    wifiManager.addParameter(&custom_mqtt_server_txt);
//    
//    WiFiManagerParameter custom_mqtt_server("server", "", conf_mqtt_server, MAX_SERVER);
//    wifiManager.addParameter(&custom_mqtt_server);
//    
//    WiFiManagerParameter custom_mqtt_port_txt("<br><br><b>MQTT: Server Port:</b>");
//    wifiManager.addParameter(&custom_mqtt_port_txt);
//    
//    WiFiManagerParameter custom_mqtt_port("port", "", conf_mqtt_port, MAX_PORT);
//    wifiManager.addParameter(&custom_mqtt_port);
//    
//    WiFiManagerParameter custom_mqtt_topic_txt("<br><br><b>MQTT: Device topic name:</b>");
//    wifiManager.addParameter(&custom_mqtt_topic_txt);
//    
//    WiFiManagerParameter custom_mqtt_topic("topic", "", conf_mqtt_topic, MAX_TOPIC);
//    wifiManager.addParameter(&custom_mqtt_topic);
    
    wifiManager.setSaveConfigCallback(saveConfigCallback); 


    // It starts an access point 
    // and goes into a blocking loop awaiting configuration.
    // Once the user leaves the portal with the exit button
    // processing will continue
    yield();
    if (!wifiManager.startConfigPortal(conf_hostname, PORTALPASS)) {
            Serial <<  F("WIFI: NO CONNECTION.") << endl;
            yield();
            flasher.detach();
            digitalWrite(STATUS_LED, LED_INVERTED); // turn led off
            
    } else {
        Serial <<  F("WIFI: Connected!") << endl;
        Serial <<  F("IP address: ") << WiFi.localIP() << endl;
    yield();
    }
//    strcpy(conf_mqtt_server, custom_mqtt_server.getValue());
//    strcpy(conf_mqtt_port, custom_mqtt_port.getValue());
//    strcpy(conf_mqtt_topic, custom_mqtt_topic.getValue());
    strcpy(conf_hostname, custom_hostname.getValue());
    #ifdef TELEGRAM
    strcpy(conf_bottoken, custom_bot_id.getValue());
    #endif
    
    Serial <<  F("PORTAL: Ending portal mode session.") << endl;
    Serial <<  F("SAVE (if needed)") << endl;
    yield();
    delay(1000);
    saveConfig();
    flasher.detach();
    
}


/// END WIFI \\\

//// BUTTON \\\

#ifdef ENABLE_BUTTON

void configurePushButton(Bounce& bouncedButton){

        // Set the debounce interval to 15ms - default is 10ms
        bouncedButton.interval(15);
}

void onButtonPressed(Button& btn){

  Serial.println("button pressed");

}

void onButtonHeld(Button& btn, uint16_t duration, uint16_t repeatCount){

  if (duration >= BUT_RESET) {
      Serial.println(F("SYS: RESET CONF & REBOOT"));
      yield();
      
      // Delete config & Reboot
      // TOdo deleteConfig();
      ESP.reset();
      delay(5000);
    
  }
}

// duration reports back the total time that the button was held down
void onButtonReleased(Button& btn, uint16_t duration){
  if (duration <= BUT_DOOR) {
      if (traped) initTrap();
       else closeTrap();
  }
  if (duration >= BUT_REBOOT && duration < BUT_CONFIG) {
      Serial.println(F("SYS: REBOOT"));
      yield();
      ESP.reset();
      delay(5000);
  }
  else if (duration >= BUT_CONFIG && duration < BUT_RESET) {
      Serial.println(F("SYS: STARTING PORTAL"));
      yield();
      startPortal();
    
  }
}

#endif

/// END BUTTON \\\

void flash() {
  
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  
}
