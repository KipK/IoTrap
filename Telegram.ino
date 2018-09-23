#ifdef TELEGRAM


// Function called on receiving a message
void onReceive (TelegramProcessError tbcErr, JwcProcessError jwcErr, Message* msg)
{      
    Serial.println("onReceive");
    Serial.print("tbcErr"); Serial.print((int)tbcErr); Serial.print(":"); Serial.println(toString(tbcErr));
    Serial.print("jwcErr"); Serial.print((int)jwcErr); Serial.print(":"); Serial.println(toString(jwcErr));
  
    Serial.print("UpdateId: "); Serial.println(msg->UpdateId);      
    Serial.print("MessageId: "); Serial.println(msg->MessageId);
    Serial.print("FromId: "); Serial.println(msg->FromId);
    Serial.print("FromIsBot: "); Serial.println(msg->FromIsBot);
    Serial.print("FromFirstName: "); Serial.println(msg->FromFirstName);
    Serial.print("FromLastName: "); Serial.println(msg->FromLastName);
    Serial.print("FromLanguageCode: "); Serial.println(msg->FromLanguageCode); 
    Serial.print("ChatId: "); Serial.println(msg->ChatId);
    Serial.print("ChatFirstName: "); Serial.println(msg->ChatFirstName);
    Serial.print("ChatLastName: "); Serial.println(msg->ChatLastName);
    Serial.print("ChatType: "); Serial.println(msg->ChatType);
    Serial.print("Text: "); Serial.println(msg->Text);
    Serial.print("Date: "); Serial.println(msg->Date);
    Serial.print("Free heap:");
    Serial.println(ESP.getFreeHeap(),DEC);
    // Sending the text of received message back to the same chat
    // and add the custom keyboard to the message
    // chat is identified by an id stored in the ChatId attribute of msg
    Serial.print("Free heap:");
    Serial.println(ESP.getFreeHeap(),DEC);  
    bot->postMessage(msg->ChatId, msg->Text);

    
}

// Function called if an error occures
void onError (TelegramProcessError tbcErr, JwcProcessError jwcErr)
{
  Serial.println("onError");
  Serial.print("tbcErr"); Serial.print((int)tbcErr); Serial.print(":"); Serial.println(toString(tbcErr));
  Serial.print("jwcErr"); Serial.print((int)jwcErr); Serial.print(":"); Serial.println(toString(jwcErr));
}


#endif
