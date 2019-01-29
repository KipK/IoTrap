IoTrap - an ESP8266 mouse trap with Telegram bot.

Needed: a servo motor, 
1 esp8266, 
1 button (optionnal)
1 IR proximity sensor like this https://www.banggood.com/TCRT5000-Infrared-Reflective-Switch-IR-Barrier-Line-Track-Sensor-Module-p-1038443.html

Additional libraries needed:

	ArduinoJson.h ( 5.13.4 ) 
	WifiManager ( 0.14.0 ) 
	TelegramBotClient ( 0.6.1 )

	Optionnal: 
	r89m Button (from https://github.com/r89m/Button)
	r89m PushButton ( https://github.com/r89m/PushButton )
	
First boot can be longer than normal as it will format spiffs partition if needed.

It will default crate a hotspot wifi with a captive portal.
There you can configure your wifi settings and add your Telegram bot api key.

Short press on button ( < 2s ) will close the door or reinitialise the trap depending of the current status.
Pressing more than 5 sec will start captive portal.
Long press of more than 10 sec will reset the device settings and reboot.

You'll prehaps need to edit the SERVOOPEN / SERVOCLOSE defines in IoTrap.ino to adjust correct servo positions.



