#include "main_definitions.h"
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "sigfox.h"

#define USE_ARDUINO_OTA true

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "masi/pub-"
#define AWS_IOT_SUBSCRIBE_TOPIC "masi/sub"

WiFiClientSecure client;
WiFiClient espclient;
MQTTClient mqtt_client = MQTTClient(1650);

uint8_t serial_rx_buffer[RX_BUFFER_SIZE];
uint16_t serial_rx_buffer_idx;
RemoteDebug Debug;
uint32_t last_time = millis();
char masi_publish_topic[30] = {'\0'};
char thing_name[30] = {'\0'};

void setup() {
    serial_mgr_init();
    data_mgr_init();
    wifi_init();

    serial_rx_buffer_idx = 0;
    memset(serial_rx_buffer, '\0', sizeof(serial_rx_buffer));
    sigfox_init();
    Mqtt_init();
    
    #ifdef TEL_DEBUG
        Tel_debug_init();
    #endif
    #ifdef OTA_ENABLED
        OTA_init(); 
    #endif
}

void loop(){
    mqtt_client.loop();
    delay(10);  // <- fixes some issues with WiFi stability
    
    if (!mqtt_client.connected()) {
        Mqtt_connect();
    }

    #ifdef OTA_ENABLED
        ArduinoOTA.handle();
    #endif

    //test_send();
    //delay(3000);
    
    if(serial_mgr_get_frame(serial_rx_buffer, &serial_rx_buffer_idx))
    {
        uint8_t cmd = serial_rx_buffer[3];
        
        //Serial.println("New data!");
        #ifdef TEL_DEBUG
            Debug.print("New data!");
        #endif    
        switch (cmd)
        {
            case 0x01:
                pfv_values_handler((char *)serial_rx_buffer);
                Clean_serial_buffer();
                break;
            case 0x02:
                measured_vals_handler((char *)serial_rx_buffer);
                Clean_serial_buffer();
                break;
            default:
                break;
        }
    }
    
    #ifdef TEL_DEBUG
        Debug.handle();
    #endif
}

void OTA_init(void){
    ArduinoOTA.onStart([]() {
        Serial.println("Start.");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd.");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}

void Mqtt_init(void){
    sprintf(masi_publish_topic, "masi/pub-%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]);
    //Serial.println("MQTT publish Topic:");
    //Serial.println(masi_publish_topic);
    mqtt_client.begin("masi.smartiotgroup.com", 1883, espclient);
    mqtt_client.onMessage(messageReceived);
    Mqtt_connect();
}

void Mqtt_connect(void){
    //Serial.print("Connecting to AWS IOT");
    uint32_t start_time = millis();
    sprintf(thing_name, "ESP8266-%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]);

    mqtt_client.setOptions(10, true, 20000);

    while (!mqtt_client.connect(thing_name) && ((millis() - start_time) <= MQTT_CONNECT_TIMEOUT) ) {
        //Serial.print(".");
        delay(100);
    }

    if(!mqtt_client.connected()){
        //Serial.println("AWS IoT Timeout!");
        return;
    }

    //Serial.println("AWS IoT Connected!");
}

void publishMessage(char *dataBuffer)
{
  mqtt_client.publish(masi_publish_topic, dataBuffer, true, 1);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void Clean_serial_buffer(void){
     memset((void *)serial_rx_buffer, 0x00, sizeof(serial_rx_buffer));
}