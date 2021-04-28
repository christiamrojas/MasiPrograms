#include "main_definitions.h"
#include "http.h"
#include "wifi.h"

byte mac[6] = {0};
char AP_name[30] = {'\0'};
IPAddress timeServerIP;
WiFiUDP udp;
byte packetBuffer[NTP_PACKET_SIZE];

unsigned long secs_since_epoch = 0;
int64_t ms_since_epoch = 0;
unsigned long offset_millis = 0;

void wifi_init(void)
{
    WiFi.macAddress(mac);
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(FIVE_MINUTES);
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    sprintf(AP_name, "Masi AP-%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]);

    wifiManager.autoConnect(AP_name);
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();
    //client.setFingerprint(fingerprint);
    udp.begin(UDP_PORT);
    //Serial.print("UDP Local port: ");
    //Serial.println(udp.localPort());
    uint32_t timeout_millis = millis();

    while((secs_since_epoch == 0) && ((millis() - timeout_millis) < ACCESS_POINT_TIMEOUT)){
       secs_since_epoch =  Wifi_get_timestamp();
       delay(1000);
    }
    offset_millis = millis();
    ms_since_epoch = int64_t(secs_since_epoch)*int64_t(1000UL);
    uint32_t first_part = ms_since_epoch & 0xFFFFFFFF;
    uint32_t second_part = (ms_since_epoch >> 32) & 0xFFFFFFFF;
    /*
    Serial.println("Milliseconds since epoch: ");
    Serial.println(first_part);
    Serial.println(second_part);
    Serial.print("Offset millis: ");
    Serial.println(offset_millis);
    */
    if(secs_since_epoch != 0){
        uint8_t cmd = 0xBC;
        uint8_t frame_to_send[4] = {0};
        uint32_t secs_peru_timezone = secs_since_epoch - 18000UL;
        frame_to_send[0] = (secs_peru_timezone & 0xFF000000) >> 24;
        frame_to_send[1] = (secs_peru_timezone & 0x00FF0000) >> 16;
        frame_to_send[2] = (secs_peru_timezone & 0x0000FF00) >> 8;
        frame_to_send[3] = (secs_peru_timezone & 0x000000FF);
        serial_mgr_pack(cmd, frame_to_send, 4);
    }
}

uint32_t Send_ntp_packet(IPAddress& address)
{
  //Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
                                  // 8 bytes of zero for Root Delay 
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket(); 
  return 0; 
}

uint32_t Wifi_get_timestamp(void){
    WiFi.hostByName(NTP_SERVER_NAME, timeServerIP); 
    WiFi.hostByName(NTP_SERVER_NAME, timeServerIP); 
    Send_ntp_packet(timeServerIP);
    delay(1000);

    int cb = udp.parsePacket();
    if(cb)
    {
        //Serial.print("packet received, length=");
        //Serial.println(cb);
        udp.read(packetBuffer, NTP_PACKET_SIZE);
    
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;        
        
        const unsigned long seventyYears = 2208988800UL;
        uint32_t epoch = secsSince1900 - seventyYears;

        //Serial.print("Seconds since Jan 1 1900 = " );
        //Serial.println(epoch);

        return epoch;
    }
    else
    {
        //Serial.println("no packet yet");
        return 0;
    }

}

void wifi_sync_timestamp(void){
    ms_since_epoch = ms_since_epoch + uint64_t(millis() - offset_millis);
}