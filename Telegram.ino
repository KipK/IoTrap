#ifdef TELEGRAM


// Function called on receiving a message
void onReceive (TelegramProcessError tbcErr, JwcProcessError jwcErr, Message* msg)
{      
    Serial << "BOT: Received msg at " << msg->Date <<  endl;
    Serial << "From: " <<  msg->FromFirstName << " " << msg->FromLastName << endl;
    Serial << "Text: " <<  msg->Text << endl;
    
//    Serial.print("tbcErr"); Serial.print((int)tbcErr); Serial.print(":"); Serial.println(toString(tbcErr));
//    Serial.print("jwcErr"); Serial.print((int)jwcErr); Serial.print(":"); Serial.println(toString(jwcErr));  
//    Serial.print("UpdateId: "); Serial.println(msg->UpdateId);      
//    Serial.print("MessageId: "); Serial.println(msg->MessageId);
//    Serial.print("FromId: "); Serial.println(msg->FromId);
//    Serial.print("FromIsBot: "); Serial.println(msg->FromIsBot);
//    Serial.print("FromFirstName: "); Serial.println(msg->FromFirstName);
//    Serial.print("FromLastName: "); Serial.println(msg->FromLastName);
//    Serial.print("FromLanguageCode: "); Serial.println(msg->FromLanguageCode); 
//    Serial.print("ChatId: "); Serial.println(msg->ChatId);
//    Serial.print("ChatFirstName: "); Serial.println(msg->ChatFirstName);
//    Serial.print("ChatLastName: "); Serial.println(msg->ChatLastName);
//    Serial.print("ChatType: "); Serial.println(msg->ChatType);
//    Serial.print("Text: "); Serial.println(msg->Text);
//    Serial.print("Date: "); Serial.println(msg->Date);
    
    String sendmsg = "";
    if (msg->Text == "/status") {
      if (traped) sendmsg = "Dude, we have a mouse here! Trap is locked.";
      else        sendmsg = "Trap is ready and waiting for a target";
      //->postMessage(msg->ChatId, sendmsg);
      
    }
    else if (msg->Text == "/network") {
        sendmsg = WiFi.localIP().toString();
    }
      
    bot->postMessage(msg->ChatId, sendmsg);
    
}

// Function called if an error occures
void onError (TelegramProcessError tbcErr, JwcProcessError jwcErr)
{
  Serial.println("onError");
  Serial.print("tbcErr"); Serial.print((int)tbcErr); Serial.print(":"); Serial.println(toString(tbcErr));
  Serial.print("jwcErr"); Serial.print((int)jwcErr); Serial.print(":"); Serial.println(toString(jwcErr));
}


#endif
