#ifdef TELEGRAM

String helpmsg = "Commands:\n"\
                 "-  /start              : subscribe to notifications\n"
                 "-  /stop               : unsubscribe to notifications\n"
                 "-  /status             : show trap status\n"
                 "-  /device             : show device infos\n"
                 "Admin Commands:\n"
                 "-  /users <PASS>       : show all users\n"
                 "-  /delusers <PASS/    : remove all users\n"
                 "-  /open <PASS>    : open trap door\n"
                 "-  /close <PASS>   : close trap door\n";
            
JsonObject& getSubscribedUsers() {
  
        
  File subscribedUsersFile = SPIFFS.open("/"+subscribed_users_filename, "r");
  delay(1);

  if (!subscribedUsersFile) {
    Serial.println("Failed to open subscribed users file");

    // Create empty file (w+ not working as expect)
    File f = SPIFFS.open("/"+subscribed_users_filename, "w");
    delay(1);
    f.close();

    JsonObject& users = jsonBuffer.createObject();
    return users;
  } else {

    size_t size = subscribedUsersFile.size();

    if (size > 1024) {
      Serial.println("Subscribed users file is too large");
      //return users;
    }

    String file_content = subscribedUsersFile.readString();
    
    JsonObject& users = jsonBuffer.parseObject(file_content);

    if (!users.success()) {
      Serial.println("Failed to parse subscribed users file");
      return users;
    }

    subscribedUsersFile.close();
    delay(1);


    return users;
  }
    
}

String listSubscribedUsers() {
  JsonObject& users = getSubscribedUsers();
  Serial << "USERS: ";
  users.printTo(Serial);
  Serial << endl;    
}

bool addSubscribedUser(long chat_id, String from_name) {
  JsonObject& users = getSubscribedUsers();
  File subscribedUsersFile = SPIFFS.open("/"+subscribed_users_filename, "w+");
  delay(1);
  if (!subscribedUsersFile) {
    Serial.println("Failed to open subscribed users file for writing");
    //return false;
  }

  users.set(String(chat_id), from_name);
  
  users.printTo(subscribedUsersFile);
  
  subscribedUsersFile.close();
  delay(1);
  Serial << " subscribeduser file closed ( addsub)" << endl;
  return true;
    
}
        
bool removeSubscribedUser(long chat_id) {
    JsonObject& users = getSubscribedUsers();
          
      File subscribedUsersFile = SPIFFS.open("/"+subscribed_users_filename, "w");
      delay(1);
    
      if (!subscribedUsersFile) {
        Serial.println("Failed to open subscribed users file for writing");
        return false;
      }
    
      users.remove(String(chat_id));
      users.printTo(subscribedUsersFile);
    
      subscribedUsersFile.close();
      delay(1);
     
      return true;
    
        
}


void sendMessageToUsers(String message) {
  int users_processed = 0;

  JsonObject& users = getSubscribedUsers();

  for (JsonObject::iterator it=users.begin(); it!=users.end(); ++it) {
    users_processed++;
        // temporary hak
        button.update();
        yield;
        
    if (users_processed < messages_limit_per_second)  {
      Serial << "Send msg to " << it->key << " msg: " << message << endl;
      bot ->postMessage(long(atoi(it->key)), message);
      yield();
    } else {
      delay(bulk_messages_mtbs);
      users_processed = 0;
    }
  }
}


void onReceive (TelegramProcessError tbcErr, JwcProcessError jwcErr, Message* msg)
{      

    long chat_id = long(msg->ChatId);
    String from_name = String(msg->FromFirstName);
    String text = String(msg->Text);
    String type = String(msg->ChatType);
    String sendmsg = ""; 

    Serial << "MSG: text: " << text << endl;
    Serial << " Chat id: " << chat_id << " | Type: " << type << endl;
    //if(type == "message") { // commands for private message only
      if (text == "/start") {
        
          if (addSubscribedUser(chat_id, from_name)) {
            sendmsg =  "Welcome " + from_name + " ,\n";
            sendmsg += "I'm IoTrap bot, an overkill but cool non lethal mouse trap.\n";
            sendmsg += "You are now SUBSCRIBED to notifications\n";
            sendmsg += helpmsg;

          } else {
            sendmsg = "Something wrong, can't subscribe, please try again (later?)\n";
          }
      }
      else if (text == "/stop") {
        if (removeSubscribedUser(chat_id)) {
          sendmsg = "Thank you " + from_name + ", you are now unsubscribed";
        } else {
          sendmsg = "Something went wrong, can't unsubscribe, please try again (later?)";
        }
      }
      else if (text == "/help") {
        sendmsg =  "==  IoTRAP - WiFi Mouse Trap  ==\n\n"; 
        sendmsg += helpmsg;
      }
      else if (text == "/status") {
        if (traped) sendmsg = "Dude, we have a 🐭 here! Trap is locked.";
        else        sendmsg = "Trap is ready and waiting for 🐭";
       
      }
      else if (text == "/device") {
          sendmsg = WiFi.localIP().toString();
      }
      else if (text == ("/users " + String(TGRM_PASS))) {
        JsonObject& users = getSubscribedUsers();
        users.printTo(sendmsg);

      }
      else if (text == ("/delusers " + String(TGRM_PASS))) {
        if (SPIFFS.remove("/"+subscribed_users_filename)) {
          sendmsg = "All users removed";
        } else {
          sendmsg = "Something went wrong, please try again (later?)";
        }
      }
      else if(text == ("/open " + String(TGRM_PASS))) {
        initTrap();
        sendmsg = "Opening trap, waiting for target";
      }
      else if(text == ("/close " + String(TGRM_PASS))) {
        closeTrap();
        sendmsg = "Closing trap";
      }

      if (sendmsg) {
        bot->postMessage(msg->ChatId, sendmsg);
        sendmsg = "";
      }
   // }
    
    
}

// Function called if an error occures
void onError (TelegramProcessError tbcErr, JwcProcessError jwcErr)
{
  Serial.println("onError");
  Serial.print("tbcErr"); Serial.print((int)tbcErr); Serial.print(":"); Serial.println(toString(tbcErr));
  Serial.print("jwcErr"); Serial.print((int)jwcErr); Serial.print(":"); Serial.println(toString(jwcErr));
}

#endif
