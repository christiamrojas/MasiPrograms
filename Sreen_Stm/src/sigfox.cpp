#include <Arduino.h>
#include "sigfox.h"

static char sigfox_rx_buf[100] = {'\0'};
static char ID[51]= {'\0'};
static char PAC[51] = {'\0'};

void sigfox_init(void){
    Serial1.begin(9600);
    pinMode(SIGFOX_ENABLE, OUTPUT);
    sigfox_change_to_region_4();
    sigfox_read_info();
}

void sigfox_change_to_region_4(void)
{
  
  digitalWrite(SIGFOX_ENABLE, HIGH);
  delay(1000);
  Serial1.print("AT$DR=922300000");
  Serial1.print("\r\n");
  Serial1.print("ATS400=<00000000><F0000000><0000001F>,63");
  Serial1.print("\r\n");
  Serial1.print("AT$WR");
  Serial1.print("\r\n");
  Serial1.print("AT$RC");
  Serial1.print("\r\n");
  digitalWrite(SIGFOX_ENABLE, LOW);
  
}

void sigfox_read_info(void)
{
  
  digitalWrite(SIGFOX_ENABLE, HIGH);
  delay(1000);
  sigfox_send_AT_command("AT");
  sigfox_send_AT_command("AT$I=10");
  strcpy(ID, sigfox_rx_buf);
  sigfox_send_AT_command("AT$I=11");
  strcpy(PAC, sigfox_rx_buf);
  sigfox_send_AT_command("AT$RC");
  digitalWrite(SIGFOX_ENABLE, LOW);
  delay(500);
  Serial.print("\n\n\n\rID:");
  Serial.println(ID);
  Serial.print("PAC:");
  Serial.println(PAC);
  
}

void sigfox_send_message(String buf_tx){
  
  //agregamos el salto de linea "\n"
  buf_tx+="\n";
  
  //Habilitamos el modulo Sigfox
  digitalWrite(SIGFOX_ENABLE, HIGH);
  delay(1000);
  //Reset del canal para asegurar que manda en la frecuencia correcta
  Serial.print("AT$RC\n"); 
  Serial1.print("AT$RC\n");
  
  //Enviamos la informacion por sigfox
  Serial.print(buf_tx);
  Serial1.print(buf_tx);
  delay(3000);
  
  //deshabilitamos el modulo Sigfox
  digitalWrite(SIGFOX_ENABLE, LOW);
  
}

void sigfox_send_AT_command(char* comandoAT){
  unsigned long x=0;
  while( Serial1.available() > 0) Serial1.read();
  Serial1.println(comandoAT);
  
  memset(sigfox_rx_buf, '\0',sizeof(sigfox_rx_buf)); 
  
  while(true){
    
    if(Serial1.available() != 0)
    {   
      sigfox_rx_buf[x] = Serial1.read();
      x++;
      if (strstr(sigfox_rx_buf, "\n") != NULL)
      {
        break;
      }
    }
  }
}