#include <SoftwareSerial.h>
#include "sigfox.h"
#include "data_mgr.h"

SoftwareSerial sw_serial(12, 14);      // RX, TX
static char sigfox_rx_buf[100] = {'\0'};
char ID[51]= {'\0'};
char PAC[51] = {'\0'};

void sigfox_init(void){
    sw_serial.begin(9600);
    pinMode(SIGFOX_ENABLE, OUTPUT);
    sigfox_change_to_region_4();
}

void sigfox_change_to_region_4(void)
{
  digitalWrite(SIGFOX_ENABLE, HIGH);
  delay(1000);
  sw_serial.print("AT$DR=922300000");
  sw_serial.print("\r\n");
  sw_serial.print("ATS400=<00000000><F0000000><0000001F>,63");
  sw_serial.print("\r\n");
  sw_serial.print("AT$WR");
  sw_serial.print("\r\n");
  sw_serial.print("AT$RC");
  sw_serial.print("\r\n");
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
  sw_serial.print("AT$RC\n"); 
  sw_serial.print("AT$RC\n");
  
  //Enviamos la informacion por sigfox
  sw_serial.print(buf_tx);
  //delay(3000);
  
  //deshabilitamos el modulo Sigfox
  //digitalWrite(SIGFOX_ENABLE, LOW);
  
}

void sigfox_send_AT_command(char* comandoAT){
  unsigned long x=0;
  while( sw_serial.available() > 0) sw_serial.read();
  sw_serial.println(comandoAT);
  
  memset(sigfox_rx_buf, '\0',sizeof(sigfox_rx_buf)); 
  
  while(true){
    
    if(sw_serial.available() != 0)
    {   
      sigfox_rx_buf[x] = sw_serial.read();
      x++;
      if (strstr(sigfox_rx_buf, "\n") != NULL)
      {
        break;
      }
    }
  }
}

void sigfox_send_active_alarms(sample_measured_vars_struct *sample_data, alarms_struct *active_alms_data){

  String sfgx_payload = "AT$SF=";
  String count_str;
  uint32_t alarm_value = 0;

  for(int idx=0; idx < ALARM_SIZE; idx++){
    if((sample_data->alarms[idx] == 1) && (active_alms_data->active_status[idx] == 0)){
      active_alms_data->active_status[idx] = 1;
      active_alms_data->active_time[idx] = millis();
      
    }
    else if(sample_data->alarms[idx] == 0)
      active_alms_data->active_status[idx] = 0;
    
    if((active_alms_data->active_status[idx] == 1) && ((millis() - active_alms_data->active_time[idx]) >= SIGFOX_ALARM_ACTIVE_TIME)){
      alarm_value |= (1 << idx);
      active_alms_data->active_time[idx] = millis();
    }
      
  }

  if(alarm_value == 0)
    return;

  uint8_t *num = (uint8_t *)&alarm_value;

  for(int i=3; i >= 0; i--){
    count_str = String(num[i], HEX);
    if(count_str.length() < 2)
      sfgx_payload += '0' + count_str;
    else
      sfgx_payload += count_str;
  }

  //Serial.println("Trama generada:");
  //Serial.println(sfgx_payload);
  sigfox_send_message(sfgx_payload);

}