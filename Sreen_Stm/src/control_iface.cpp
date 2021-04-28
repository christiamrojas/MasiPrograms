#include "control_iface.h"

void control_iface_init(void)
{
  //Code that depends on device used
  // Serial2.begin(BAUDRATE);
  Serial3.begin(BAUDRATE);
}

void control_iface_write_frame(uint8_t *dataFrame, uint8_t frameLen)
{
  for(int i=0; i<frameLen; i++)
  {
    control_iface_write_byte(dataFrame[i]);
    #ifdef PRINT_CTL_IFACE_OUTPUT 
    Serial.print(dataFrame[i], HEX); Serial.print(" ");
    #endif
  }
  #ifdef PRINT_CTL_IFACE_OUTPUT 
  Serial.println("");
  #endif
}

void control_iface_write_byte(uint8_t dataByte)
{
  //Code that depends on device used
  // Serial2.write(dataByte);
  Serial3.write(dataByte);
}

bool control_iface_available()
{
  //Code that depends on device used
  // if(Serial2.available() > 0)
  if(Serial3.available() > 0)
  {
  return true;  
  }

  return false;
}

uint8_t control_iface_read()
{
  //Code that depends on device used
  //  return Serial2.read();
  return Serial3.read();
}

bool control_iface_get(uint8_t *dataByte)
{
  if(!control_iface_available())
  {
    return false;
  }
  *dataByte = control_iface_read();
  return true;
}

void control_iface_pack(uint8_t cmd, uint8_t *dataFrame)
{
  uint8_t txBuffer[TXBUFFER_MAXLEN];

  txBuffer[0] = HEADER1;
  txBuffer[1] = HEADER2;

  switch(cmd)
  {
    case 0xB1: // Stop + <Parametros>: 0x00->Parada terminando el ciclo, 0x01->Pausa, 0x02->Reanudar
      txBuffer[2] = 2;
      txBuffer[3] = cmd;
      txBuffer[4] = dataFrame[0];
      control_iface_write_frame(txBuffer, 5); //len = 3 + contents of txBuffer[2]
      break;
    case 0xB2: // Start VC-CMV + <Volumen_ml> + <Flujo_lm> + <T_inspiracion_dinamica_cs> + <Pausa_cs> + <T_expiracion_cs> + <Trigger_lm> + <fio2_%>
                // uint16_t x7
      txBuffer[2] = 15;
      txBuffer[3] = cmd;
      for(int i=0; i<14; i++)
      {
        txBuffer[i+4] = dataFrame[i];
      }
      control_iface_write_frame(txBuffer, 18); //len = 3 + contents of txBuffer[2]
      break;
    case 0xB3: // Start VC-CMV + <Presion_Control_cmh2o> + <T_inspiracion_dinamica_cs> + <T_expiracion_cs> + <Trigger_lm>  + <fio2%>
                // uint16_t x5
      txBuffer[2] = 11;
      txBuffer[3] = cmd;
      for(int i=0; i<10; i++)
      {
        txBuffer[i+4] = dataFrame[i];
      }
      control_iface_write_frame(txBuffer, 14); //len = 3 + contents of txBuffer[2]
      break;
    case 0xB4:// Start VC-CMV + <Presion_soporte_cmh2o> + <ciclado_%> + <Trigger_lm> + <fio2_%> + <Tiempo_Apnea_cs>
                // uint16_t x5
      txBuffer[2] = 11;
      txBuffer[3] = cmd;
      for(int i=0; i<10; i++)
      {
        txBuffer[i+4] = dataFrame[i];
      }
      control_iface_write_frame(txBuffer, 14); //len = 3 + contents of txBuffer[2]
      break;      
    case 0xB5:
      txBuffer[2] = 2;
      txBuffer[3] = cmd;
      txBuffer[4] = 0x55;
      control_iface_write_frame(txBuffer, 5);
      break;
    case 0xB6:
      txBuffer[2] = 2;
      txBuffer[3] = cmd;
      txBuffer[4] = dataFrame[0];
      control_iface_write_frame(txBuffer, 5);
      break;
    case 0xB7:
      txBuffer[2] = 1;
      txBuffer[3] = cmd;
      control_iface_write_frame(txBuffer, 4);
    case 0xB8:
      txBuffer[2] = 2;
      txBuffer[3] = cmd;
      txBuffer[4] = dataFrame[0];
      control_iface_write_frame(txBuffer, 5);
      break;
    case 0xB9:
      txBuffer[2] = 1;
      txBuffer[3] = cmd;
      control_iface_write_frame(txBuffer, 4);
      break;
    default:
      break;
  }

  return;
}

bool control_iface_check_data(uint8_t* dataBuffer, uint8_t* index)
{
  long t_start, t_elapsed;
  int timeoutCounter = 0;

  t_start = millis();
  t_elapsed = 0;

  if(control_iface_available())
  {
    #ifdef PRINT_CTL_IFACE_INPUT
    Serial.print("Incoming values: ");
    #endif
    uint8_t i=0;
    bool endFlag = 0;
    uint8_t packetLen = 0;

    while(endFlag != 1)
    {
      if(t_elapsed >= RXTIMEOUT)
      {
        return false;
      }

      if(control_iface_available())
      {
        dataBuffer[i] = control_iface_read();
        #ifdef PRINT_CTL_IFACE_INPUT
        Serial.print(dataBuffer[i], HEX); Serial.print(" "); 
        #endif
        i++;
      }

      if(i == 1)
      {
        if(dataBuffer[0] != HEADER1)
        {
          i--;
        }
      }
      else if(i == 2)
      {
        if(dataBuffer[1] != HEADER2)
        {
          i--;
        }
      }
      else if(i == 3)
      {
        packetLen = dataBuffer[2];
      }
      else if(packetLen != 0 && i >= (packetLen + 3))
      {
        endFlag = 1;
      }

      t_elapsed = millis() - t_start;
      timeoutCounter++;
    }

    *index = i;
    #ifdef PRINT_CTL_IFACE_INPUT
    Serial.println("");
    #endif
    return true;
  }
  return false;
}

uint8_t control_iface_get_cmd(uint8_t* dataBuffer, uint8_t bufferIndex)
{
  uint8_t payloadLen;
  uint8_t cmd;

  payloadLen = dataBuffer[2];
  cmd = dataBuffer[3];

  if((dataBuffer[0] != 0x5A) || (dataBuffer[1] != 0xA5))
  {
    return 0;
  }
  
  if ((payloadLen != (bufferIndex - 3)))
  {
    return 0;
  }

  return cmd;
}
