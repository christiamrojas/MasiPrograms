#ifndef MAIN_DEFINITIONS_H
#define MAIN_DEFINITIONS_H

#include "Arduino.h"
#include "wifi.h"
#include "serial_mgr.h"
#include "data_mgr.h"
#include "http.h"
#include "RemoteDebug.h"
#include "telnet.h"
#include "config.h"

// Use WiFiClientSecure class to create TLS connection
//WiFiClient client;
extern WiFiClientSecure client;
extern WiFiClient espClient;
extern RemoteDebug Debug;


#define MQTT_CONNECT_TIMEOUT 3000  //Units: ms

//Pinout de la tarjeta de Christiam
#define SIGFOX_TX   14
#define SIGFOX_RX   12
#define LED          2
#define SIGFOX_RST  13

void Mqtt_init(void);
void Mqtt_connect(void);
void OTA_init(void);
void connectAWS(void);
void publishMessage(void);
void messageHandler(String &topic, String &payload);
void messageReceived(String &topic, String &payload);
void publishMessage(char *dataBuffer);
void Clean_serial_buffer(void);

#endif