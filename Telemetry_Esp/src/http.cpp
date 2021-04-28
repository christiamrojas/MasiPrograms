#include "Arduino.h"
#include "main_definitions.h"
#include "http.h"

String line;
char fingerprint[65] = {"3D:15:7A:99:4E:C3:DF:A2:D1:41:6F:E1:7A:DF:AD:1A:84:F8:27:82"};



int http_post_request(char *dataBuffer){
  
  if (!client.connect(HOST_NAME, HOST_PORT)) 
  {
    Serial.println("connection failed");
    #ifdef TEL_DEBUG
        Debug.println("connection failed");
    #endif
    return 0;
  }

  //pubSubClient.loop();
  int result = 0;

  //Para HTTPS, descomentar esto 
  /*
  if (client.verify(fingerprint, HOST_NAME)) {
    Serial.println("certificate matches");
    #ifdef TEL_DEBUG
      Debug.println("certificate matches");
    #endif
  } else {
    Serial.println("certificate doesn't match");
    #ifdef TEL_DEBUG
      Debug.println("certificate doesn't match");
    #endif
  }
  */
  Serial.println(URL);
  
  client.println(String("POST ") + URL + " HTTP/1.1");
  client.print("Host: ");
  client.println(HOST_NAME);
  client.print("Content-Length: ");
  client.println(strlen(dataBuffer));
  client.println("Content-Type: application/json");
  client.println();
  client.println(dataBuffer);

  Serial.println("request sent");

  int lineIndex = 0;
  
  while (client.connected()) {
    //pubSubClient.loop();
    
    line = client.readStringUntil('\n');
    Serial.print("line index: ");
    Serial.print(lineIndex);
    Serial.print(" Line: ");
    Serial.println(line);

    #ifdef TEL_DEBUG
      Debug.print("line index: ");
      Debug.print(lineIndex);
      Debug.print(" Line: ");
      Debug.println(line);
    #endif

    if(line.indexOf("200") > 0)
    {
      Serial.println("Server response was OK");
      #ifdef TEL_DEBUG
        Debug.println("Server response was OK");
      #endif
      result = 1;
    }

    lineIndex++;
    yield();
    
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }

    if(lineIndex > 10)
    {
      break;
    }
  }
  
  //String line = client.readString();
  Serial.println("reply was:");
  Serial.println("==========");
  //Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  //pubSubClient.loop();
  delay(3000);

  return result;
}