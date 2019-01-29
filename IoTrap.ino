// IoTrap, an esp8266 based mouse trap.
// by Guillaume Sartre 
#define VERSION "0.1.1"



#define SERVOPIN  5
#define IRPIN     4
#define BUTTONPIN 12

#define ENABLE_BUTTON
// Button press timings
#define BUT_RESET     10000
#define BUT_CONFIG    5000
#define BUT_DOOR      2000

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
#define TGRM_LTIME 5000  // scan for new message looptime
#define TGRM_TKN_LTH 46  // Telegram token length
#define TGRM_PASS ""

//extern "C" {
//#include "user_interface.h"

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
//#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce-Arduino-Wiring


#ifdef TELEGRAM
#include <WiFiClientSecure.h>
#include <TelegramBotClient.h>
#endif


DynamicJsonBuffer jsonBuffer;
PushButton button = PushButton(BUTTONPIN,PRESSED_WHEN_LOW);
Servo servo; 
Ticker flasher;


#ifdef TELEGRAM
WiFiClientSecure client;
TelegramBotClient *bot;

#endif

bool trapped           = false;
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
int bulk_messages_mtbs = 1500; // mean time between send messages, 1.5 seconds
int messages_limit_per_second = 25; // Telegram API have limit for bulk messages ~30 messages per second
String subscribed_users_filename = "subscribed_users.json";
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
  
  if (SPIFFS.begin()) {
        Serial <<  F("Mounted SPIFFS file system") << endl;
        yield();
  } else {
          Serial <<  F("Failed to mount SPIFFS file System, format,please wait ...") << endl;
          yield();
          SPIFFS.format();
          yield();
          hasConf = false;
 
  }

  loadConfig(); //load configuration data 
  
  Serial << "Connecting Wifi..." << endl;
  wifi_station_set_hostname(conf_hostname); //Set hostname
  startWifi(); 
  

  // Init IR sensor
  // ToDO...

  // Start Telegram bot
  #ifdef TELEGRAM

  // Instantiate the client with secure token and client
  bot = new TelegramBotClient( conf_bottoken, client);
  #endif
  
   bot->begin(      
      onReceive,
      onError);    
  initTrap(); // Open the door at boot
  String sendmsg= "";
  sendmsg =  "☠🐭☠ -- " + String(conf_hostname) + " " + VERSION + " -- ☠🐭☠\n Ready & Armed.\n Waiting for 🐭🐭🐭🐭..." ;
  sendMessageToUsers(sendmsg);
     
   
}

void loop() {
  static long wifi_ltime = millis(); // Wifii looptime
  static long main_ltime = millis();  // Main looptime
  static long trap_ltime = 0; // for how long mouse has been traped 
  
  #ifdef ENABLE_BUTTON
  // Check the state of the button
  button.update();
  #endif

  if (!hasConf) {
    startPortal();
  }

  if ((millis() - main_ltime) >= 100) { 
    // Check if we have a mouse around
    if (!trapped){
      if(spotMouse()) {
      // Mouse spotted, close trap     
         closeTrap();
         sendMessageToUsers("We've trapped a mouse here!");
         trap_ltime = 1;
      }
    }
    else {
      if (trap_ltime) {
          trap_ltime++;
          if (trap_ltime >= 20) {
            trap_ltime = 0;
            openTrap(); // repositionning servo to open position after 2s to prevent mouse eating the hatch.
          }
      }
    }
   
    main_ltime = millis();
  }
  else if ((millis() - main_ltime) < 0) main_ltime = millis(); // in case it overflown
 

  if(WiFi.status() != WL_CONNECTED) {
    // NO CONNECTION   
      if ((millis() - wifi_ltime) >= 10000) {
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
    bot->loop();
    #endif
    
  }
}


bool spotMouse() {
  if (!dbDigitalRead(IRPIN)) {
    return true; // mouse spotted
  }
  else return false;
}

void closeTrap() {
  Serial << "Closing trap" << endl;
  servo.attach(SERVOPIN);
  delay(10);
  servo.write(SERVOCLOSE);
  delay(100);
  servo.detach();
  trapped = true;
}

void openTrap() {
  servo.attach(SERVOPIN);
  delay(10);
  servo.write(SERVOOPEN);
  delay(100);
  servo.detach();
}

void initTrap() {
  Serial << "Opening Trap" << endl;
  trapped = false;
  openTrap();
}
