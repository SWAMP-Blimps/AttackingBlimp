#include "SerialHandler.h"

void SerialHandler::Init(){
    Serial.begin(115200);
}

void SerialHandler::Update(){
    // Check for lost serial connection
    float currentMillis = millis();
    if(connectedSerial && (currentMillis-lastHeartbeatMillis)/1000.0 >= timeout_serial){
        // Lost serial connection
        connectedSerial = false;
        if(callback_SerialDisconnect) callback_SerialDisconnect();
    }

    // Read in messages
    ParseMessages();

    // Send out messages
    SendMessages();
}

void SerialHandler::SendSerial(char flag, String message){
    // Add message to bufferSerial_out
    bufferSerial_out += flag;
    bufferSerial_out += message;
    // bufferSerial_out += 'L';
    // bufferSerial_out += char(message.length());
    bufferSerial_out += delimiter_serial;
    

    // TO-DO: ADD A WAY TO DETECT IF OUR BUFFER GETS TOO BIG (DATA IS LOSING)
}

// Parses messages along Serial
void SerialHandler::ParseMessages(){
    while(Serial.available() > 0){
        char currentChar = Serial.read();
        RecordSerialHeartbeat();
        if(currentChar != delimiter_serial){
            bufferSerial_in += currentChar;
        }else{
            String message = bufferSerial_in;
            bufferSerial_in = "";
            ParseMessage(message);
        }
    }
}

void SerialHandler::ParseMessage(String message){
    if(callback_SerialRecvMsg) callback_SerialRecvMsg(message);
}

void SerialHandler::SendMessages(){
    if(bufferSerial_out.length() == 0) return;

    if(bytesPerMessage <= 0 || messagesPerSecond <= 0){
        // Send entire buffer immediately
        Serial.print(bufferSerial_out);
        bufferSerial_out = "";
    }else{
        // Check if it is time to send another message
        float currentTimeMicros = micros();
        if(currentTimeMicros - lastMessageOutMicros >= microsPerSecond/messagesPerSecond){
            lastMessageOutMicros = currentTimeMicros;

            // Consider if the buffer is shorter than max message length
            int lastIndex = bytesPerMessage < bufferSerial_out.length() ? bytesPerMessage : bufferSerial_out.length();
            String message = bufferSerial_out.substring(0, lastIndex);
            bufferSerial_out = bufferSerial_out.substring(lastIndex);
            Serial.print(message);
        }
    }
}

void SerialHandler::RecordSerialHeartbeat(){
    lastHeartbeatMillis = millis();
    if(!connectedSerial){
        // New serial connection
        connectedSerial = true;
        if(callback_SerialConnect) callback_SerialConnect();
    }
}