// IoTrap, an esp8266 based mouse trap.
// by Guillaume Sartre 




#define SERVOPIN  5
#define IRPIN     4
#define BUTTONPIN 12

#define ENABLE_BUTTON
// Button press timings
#define BUT_RESET     10000
#define BUT_CONFIG    5000
#define BUT_REBOOT    2000
#define BUT_DOOR      500

#define STATUS_LED                   2     // Built-in blue LED on pin 2
#define LED_INVERTED                 1     // 1 = High: Off, Low: On, 0 = opposite
  
#define SERVOOPEN  90 // servo value when door is open
#define SERVOCLOSE 60 //

#define HOSTNAME                 "IoTrap-"                // Default host when note configured
#define PORTALPASS               "password"                 // Set AP wifi password for portal mode

#define MAX_SERVER  64
#define MAX_PORT    5
#define MAX_TOPIC   64
#define MAX_PATH    128
#define MAX_HOST    64

#define TELEGRAM                                            // enable Telegram bot
#define TGRM_LTIME 2000  // scan message each 2 sec.
#define TGRM_TKN_LTH 46  // Telegram token length
#define TGRM_CHAN        // Telegram channel to post

#include "streamprint.h"                                    // Stream to Serial: Serial << "blahblah" << endl;
#include <FS.h>


#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>                                      // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>                               // Local WebServer used to serve the configuration portal
#include <WiFiManager.h>                                    // https://github.com/kentaylor/WiFiManager WiFiManager forked from tzapu
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <Servo.h>
#include <PushButton.h>
#include <Button.h>
#include <ButtonEventCallback.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce-Arduino-Wiring


#ifdef TELEGRAM
#include <WiFiClientSecure.h>
#include <TelegramBotClient.h>
#endif



PushButton button = PushButton(BUTTONPIN,PRESSED_WHEN_LOW);
Servo servo; 
Ticker flasher;

#ifdef TELEGRAM
WiFiClientSecure sslPollClient;
TelegramBotClient *bot;
// Instantiate a keybord with 3 rows
TBCKeyBoard board(3);
#endif

bool traped           = false;
bool shouldSaveConfig = false;
bool  hasConf         = false;
//char  conf_mqtt_server[MAX_SERVER]                = "my.mqttserver.io";  //default mosquito IP/host
//char  conf_mqtt_port[MAX_PORT]                    = "1883";
//char  conf_mqtt_topic[MAX_TOPIC]                  = "mqttnodes_/";
char  conf_hostname[MAX_HOST]                     = "IoTrap";

#ifdef TELEGRAM
char conf_bottoken[TGRM_TKN_LTH] = "";
long bot_lasttime = 0;   //last time messages' scan has been done
bool tgrm_ok = false;
#endif


void setup() {
  // put your setup code here, to run once:
  pinMode(SERVOPIN,OUTPUT);
  digitalWrite(SERVOPIN, HIGH);
  pinMode(IRPIN,INPUT);
  pinMode(BUTTONPIN,INPUT);
  Serial.begin(115200);
  Serial << "IoTRAP Starting" << endl;

  pinMode(STATUS_LED, OUTPUT); // set led
  digitalWrite(STATUS_LED, LED_INVERTED); // turn led off
  
  #ifdef ENABLE_BUTTON
  button.configureButton(configurePushButton);
  button.onPress(onButtonPressed);
  button.onHoldRepeat(1000, 500, onButtonHeld);
  button.onRelease(onButtonReleased);      
  #endif

  loadConfig(); //load configuration data 
  
  Serial << "Connecting Wifi..." << endl;
  wifi_station_set_hostname(conf_hostname); //Set hostname
  startWifi(); 
  initTrap(); // Open the door at boot

  // Init IR sensor
  // ToDO...

  // Start Telegram bot
  #ifdef TELEGRAM

  // Instantiate the client with secure token and client
  bot = new TelegramBotClient( conf_bottoken, sslPollClient);
      


  // Adding the 3 rows to the keyboard
  String row1[] = {"A1", "A2"};
  String row2[] = {"B1", "B2" , "B3", "B4"};
  String row3[] = {"C1", "C2" , "C3"};

  // push() always returns the keyboard, so pushes can be chained 
  board
    .push(2, row1)
    .push(4, row2)
    .push(3, row3);
  
  // Sets the functions implemented above as so called callback functions,
  // thus the client will call this function on receiving data or on error.
  bot->begin( onReceive, onError);    
#endif
}

void loop() {
  static long wifi_ltime = millis(); // Wifii looptime
  static long main_ltime = millis();  // Main looptime
  #ifdef ENABLE_BUTTON
  // Check the state of the button
  button.update();
  #endif

  if (!hasConf) {
    startPortal();
  }

  if ((millis() - main_ltime) >= 20) { // loop each 20ms ( 50 hz ) 
    // To Do: check IR sensor status
    main_ltime = millis();
  }
  else if ((millis() - main_ltime) < 0) main_ltime = millis(); // in case it overflown
 

  if(WiFi.status() != WL_CONNECTED) {
    // NO CONNECTION   
      if ((millis() - wifi_ltime) >= 5000) {
        Serial <<  F("No Wifi, Reconnecting ...") << endl;
        yield();
        wifi_ltime = millis();
      }
      else if ((millis() - wifi_ltime) < 0) wifi_ltime = millis(); // in case it overflown
  }
  else {
    // CONNECTED DO WIFI STUFF
    //TELEGRAM
    #ifdef TELEGRAM
      if (tgrm_ok) {
        bot->loop();
      }
    #endif
    
  }
}



void closeTrap() {
  Serial << "Closing trap" << endl;
  servo.attach(SERVOPIN);
  delay(10);
  servo.write(SERVOCLOSE);
  delay(100);
  servo.detach();
  traped = true;

//  #ifdef TELEGRAM
//  bot->sendMessage(chat_id, "Mouse catched!", "");
//  #endif
}


void initTrap() {
  Serial << "Opening Trap" << endl;
  traped = false;
  servo.attach(SERVOPIN);
  delay(10);
  servo.write(SERVOOPEN);
  delay(100);
  servo.detach();
//  #ifdef TELEGRAM
//  bot->sendMessage(chat_id, "Trap ready, waiting for new target...", "");
//  #endif
}
