#ifdef TELEGRAM

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

bool addSubscribedUser(String chat_id, String from_name) {
  JsonObject& users = getSubscribedUsers();
  File subscribedUsersFile = SPIFFS.open("/"+subscribed_users_filename, "w+");
  delay(1);
  if (!subscribedUsersFile) {
    Serial.println("Failed to open subscribed users file for writing");
    //return false;
  }

  users.set(chat_id, from_name);
  
  users.printTo(subscribedUsersFile);
  
  subscribedUsersFile.close();
  delay(1);
  Serial << " subscribeduser file closed ( addsub)" << endl;
  return true;
    
}
        
bool removeSubscribedUser(String chat_id) {
    JsonObject& users = getSubscribedUsers();
          
      File subscribedUsersFile = SPIFFS.open("/"+subscribed_users_filename, "w");
      delay(1);
    
      if (!subscribedUsersFile) {
        Serial.println("Failed to open subscribed users file for writing");
        return false;
      }
    
      users.remove(chat_id);
      users.printTo(subscribedUsersFile);
    
      subscribedUsersFile.close();
      delay(1);
     
      return true;
    
        
}

void handleNewMessages(int numNewMessages) {
  Serial << "BOT: Received " << String(numNewMessages) << " new message(s)" <<  endl;
  for(int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot->messages[i].chat_id);
    String chat_title = String(bot->messages[i].chat_title);
    String type = String(bot->messages[i].type);
    String from_name = "";
    if (type == "channel_post") from_name = String(bot->messages[i].chat_title);
    else from_name = String(bot->messages[i].from_name);
    String text = bot->messages[i].text;
    String sendmsg = "";

    Serial << "MSG: text: " << text << endl;
    Serial << " Chat id: " << chat_id << " | Chat title: " << chat_title << " Type: " << type << endl;
    
    if (text == "/start") {
      if (addSubscribedUser(chat_id, from_name)) {
        sendmsg = "Welcome to the connected mouse trap " + from_name + ".\n";
        sendmsg = "You are now subscribed\n";
        sendmsg += "Here are the commands.\n\n";
        sendmsg += "/status      : show trap status\n";
        sendmsg += "/network     : show network infos\n";
        sendmsg += "/showusers   : show all subscribed users\n";
        sendmsg += "/removeusers : remove all subscribed users\n";
        sendmsg += "/stop        : unsubscribe from bot\n";
      } else {
        sendmsg = "Something wrong, can't subscribe, please try again (later?)\n";
      }
    }
    else if (text == "/status") {
      if (traped) sendmsg = "Dude, we have a mouse here! Trap is locked.";
      else        sendmsg = "Trap is ready and waiting for a target";
     
    }
    else if (text == "/network") {
        sendmsg = WiFi.localIP().toString();
    }
    else if (text == "/stop") {
      if (removeSubscribedUser(chat_id)) {
        sendmsg = "Thank you " + from_name + ", you are now unsubscribed";
      } else {
        sendmsg = "Something wrong, can't unsubscribe, please try again (later?)";
      }
    }
    else if (text == "/users") {
        JsonObject& users = getSubscribedUsers();
        users.printTo(sendmsg);

    }

      
    bot->sendMessage(chat_id, sendmsg, "Markdown");
  }

}


void sendMessageToUsers(String message) {
  int users_processed = 0;

  JsonObject& users = getSubscribedUsers();

  for (JsonObject::iterator it=users.begin(); it!=users.end(); ++it) {
    users_processed++;

    if (users_processed < messages_limit_per_second)  {
      const char* chat_id = it->key;
      Serial << "Send msg to " << chat_id << " msg: " << message << endl;
      bot ->sendMessage(chat_id, message, "");
      yield();
    } else {
      delay(bulk_messages_mtbs);
      users_processed = 0;
    }
  }
}




#endif
