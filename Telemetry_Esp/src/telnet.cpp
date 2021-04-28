#include <ESP8266mDNS.h>
#include "RemoteDebug.h"
#include "main_definitions.h"
#include "telnet.h"



void Tel_debug_init(void){
    if (MDNS.begin(DNS_HOST_NAME))
    {
        //Serial.print("* MDNS responder started -> Hostname -> ");
        //Serial.println(DNS_HOST_NAME);
    }
    
    MDNS.addService("telnet", "tcp", 23);
    
    //Initialize the telnet server of remoteDebug
    Debug.begin(DNS_HOST_NAME); //Initialize the telnet server
    Debug.setResetCmdEnabled(true); //Enable the reset command
}