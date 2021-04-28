#include "screen_driver.h"

static uint8_t headers[2] = {HEADER1, HEADER2};
static uint8_t rxBuffer[RXBUFFER_MAXLEN];
static uint32_t screen_initial_epoch_time = 0;

void screen_init(void)
{
  //Code that depends on device used 
  Serial1.begin(BAUDRATE);  
}

long screen_get_time()
{
  //Code that depends on device used
  return millis();
}

void screen_write_byte(uint8_t dataByte)
{
  //Code that depends on device used
  Serial1.write(dataByte);
}

bool screen_available(void)
{
  //Code that depends on device used
  if(Serial1.available() > 0)
  {
    return true;  
  }

  return false;
}

uint8_t screen_read(void)
{
  //Code that depends on device used
  return Serial1.read();
}


void screen_write_frame(uint8_t *dataFrame, uint8_t dataLen)
{
  for(int i=0; i<dataLen; i++)
  {
    screen_write_byte(dataFrame[i]);  
  }
}

bool screen_read_byte(uint8_t *dataByte)
{
  long t_start, t_elapsed;
  t_start = screen_get_time();
  do
  {
    if(screen_available())
    {
      *dataByte = screen_read();
      return true;
    }
    t_elapsed = screen_get_time() - t_start;
  } while(t_elapsed > 0 && t_elapsed < TIMEOUT_MS);
  return false;
}

bool screen_read_frame_full(uint8_t *dataBuffer, uint8_t* index)
{
  long t_start, t_elapsed;
  int timeoutCounter = 0;

  t_start = screen_get_time();
  t_elapsed = 0;

  if(screen_available())
  {
    uint8_t i=0;
    bool endFlag = 0;
    uint8_t packetLen = 0;

    while(endFlag != 1)
    {
      if(t_elapsed >= TIMEOUT_MS)
      {
        return false;
      }

      if(screen_available())
      {
        dataBuffer[i] = screen_read();
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
      t_elapsed = screen_get_time() - t_start;
      timeoutCounter++;
    }
    *index = i;
    return true;
  }

  return false;
}

void screen_flush_rx_buffer(void)
{
  while(screen_available())
  {
    screen_read();
  }
}

bool screen_read_frame(uint8_t frameLen)
{
  uint8_t dataByte, n1;

  while(1)
  {
    if(!screen_read_byte(&dataByte))
    {
      return false;
    }

    if(dataByte!= headers[0])
    {
      continue;
    }
    if(!screen_read_byte(&dataByte))
    {
      return false;
    }
    if(dataByte != headers[1])
    {
      continue;
    }
    break;
  }

  if(!screen_read_byte(&n1))
  {
    return false;
  }

  for(int i=0; i<n1; i++)
  {
    if(!screen_read_byte(&dataByte))
    {
      return false;
    }
    if(i < RXBUFFER_MAXLEN)
    {
      rxBuffer[i] = dataByte;
    }
  }

  if(n1 != frameLen)
  {
    return false;
  }

  return true;
}

void screen_write_reg(uint8_t reg, uint8_t *dataFrame, uint8_t frameLen)
{
  screen_write_frame(headers, 2);
  screen_write_byte(2+frameLen);
  screen_write_byte(0x80);
  screen_write_byte(reg);
  screen_write_frame(dataFrame, frameLen);
  
  delay(10);

  screen_read_ack();  
}

int screen_read_reg(uint8_t reg, uint8_t *dataFrame, uint8_t frameLen)
{
  
  screen_write_frame(headers, 2);
  screen_write_byte(0x03);
  screen_write_byte(0x81);

  screen_write_byte(reg);
  screen_write_byte(frameLen);
  delay(10); //Delay solo para el RTC
  
  if(!screen_read_frame(frameLen + 3))
  // if(!screen_read_frame_full(rxBuffer, ))
    return 0;
  if(rxBuffer[0] != 0x81)
    return 0;
  if(rxBuffer[1] != reg)
    return 0;
  if(rxBuffer[2] != frameLen)
    return 0;
  for(int i=0; i<frameLen; i++)
  {
    dataFrame[i] = rxBuffer[i+3];
  }
  return frameLen;
}

void screen_write_sram(uint16_t reg, uint16_t *data, uint8_t dataLen)
{ 
  screen_write_byte(HEADER1);
  screen_write_byte(HEADER2);
  screen_write_byte(2*dataLen + FIXED_PAYLOAD_SIZE);     // Data length (including command, data and checksum) in words
  screen_write_byte(0x82);              // Command (Write data in designated addresses in register)

  screen_write_byte((reg >> 8) & 0xFF);
  screen_write_byte(reg & 0xFF);

  for(int i=0; i<dataLen; i++)
  {
    screen_write_byte((data[i] >> 8) & 0xFF);
    screen_write_byte(data[i] & 0xFF);
  }

  delay(5);

  screen_read_ack();  
}

uint8_t screen_read_sram(uint16_t reg, uint8_t *data, uint8_t txDataLen)
{
  uint8_t responIndex = 0;
  uint8_t dataLen = txDataLen;
  bool dataInBuffer = false;
  uint32_t timeoutWindowStart;
  uint32_t timeElapsed;
  screen_write_byte(HEADER1);
  screen_write_byte(HEADER2);
  screen_write_byte(4);
  screen_write_byte(0x83);

  screen_write_byte((reg >> 8) & 0xFF);
  screen_write_byte(reg & 0xFF);
  screen_write_byte(dataLen);
  
  timeoutWindowStart = millis();
  timeElapsed = 0;
  
  while(timeElapsed < READ_SRAM_TIMEOUT_MS)
  {
    dataInBuffer = screen_read_frame_full(rxBuffer, &responIndex);
    if(dataInBuffer)
      break;
    delay(1);
    timeElapsed = millis() - timeoutWindowStart;
  }

  if(!dataInBuffer)
    return 0;

  if(rxBuffer[3] != 0x83)
    return 0;

  for(int i=0; i<rxBuffer[2]; i++)
  {
    data[i] = rxBuffer[i+4];
  }
  return dataLen;
}

void screen_write_graph(uint8_t ch_mode, uint16_t *data, uint8_t dataLen)
{
  screen_write_byte(HEADER1);
  screen_write_byte(HEADER2);
  screen_write_byte(2*dataLen + 2);             // Data length (including command, data and checksum)
  screen_write_byte(0x84);                      // Command (Write data in designated addresses in register)

  screen_write_byte(ch_mode);                  // CH_Mode defines channels for trend curve

  for(int i=0; i<dataLen; i++)
  {
    screen_write_byte((data[i] >> 8) & 0xFF);
    screen_write_byte(data[i] & 0xFF);
  }
  screen_read_ack();
}

void view_write_graph_single(uint16_t datapoint, uint8_t channel)
{
  uint8_t data[2];
  uint8_t txBuffer[7];
  uint8_t packetLen;

  packetLen = 4;  //Fixed for single datapoint in single real time graph

  data[0] = (datapoint >> 8) & 0xFF;
  data[1] = datapoint & 0xFF;

  txBuffer[0] = 0x5A;
  txBuffer[1] = 0xA5;
  txBuffer[2] = packetLen;
  txBuffer[3] = 0x84; //Command for Dynamic Trend Curve
  txBuffer[4] = channel;
  txBuffer[5] = data[0];
  txBuffer[6] = data[1];

  screen_write_frame(txBuffer,7);

  screen_read_ack();
}

uint16_t screen_read_button(void)
{
  uint8_t rxBuffer[10];
  uint8_t rxIndex;

  if(!screen_available())
    return 0;

  if(!screen_read_frame_full(rxBuffer, &rxIndex))
    return 0;
   if((rxBuffer[3] == 0x82) && (rxBuffer[4] == 0x4F) && (rxBuffer[5] == 0x4B))
    return ACK;

  return ((uint16_t)(rxBuffer[7]<<8) | rxBuffer[8]);
}

void screen_beep_buzzer(uint8_t times_10ms){
  screen_write_reg(0x02, &times_10ms, 1);
}

void screen_set_rtc_time(void){
  uint16_t dataBuffer[8];
  delay(3000);

  dataBuffer[0] = 0x5AA5;
  dataBuffer[1] = 0x1407; //year-month
  dataBuffer[2] = 0x1415; //day-hour
  dataBuffer[3] = 0x0B00; //minute-second
  screen_write_sram(0x009C, dataBuffer, 4);

}

void screen_set_rtc(screenRtc_struct screenRtc)
{
  uint16_t dataBuffer[8];
  delay(500); //reducing delay, although should be checked if it is required
  
  screenRtc.year = screenRtc.year - 2000;

  #ifdef COMMS_INPUT_PRINT
  Serial.println("Got the following info");
  Serial.print("Year: "); Serial.println(screenRtc.year);
  Serial.print("Month: "); Serial.println(screenRtc.month);
  Serial.print("Day: "); Serial.println(screenRtc.day);
  Serial.print("Hour: "); Serial.println(screenRtc.hour);
  Serial.print("Minute: "); Serial.println(screenRtc.minute);
  Serial.print("Second: "); Serial.println(screenRtc.second);
  #endif

  dataBuffer[0] = 0x5AA5;
  dataBuffer[1] = ((screenRtc.year) << 8) | (uint16_t)(screenRtc.month);
  dataBuffer[2] = (uint16_t)((screenRtc.day) << 8) | (uint16_t)(screenRtc.hour);
  dataBuffer[3] = (uint16_t)((screenRtc.minute) << 8) | (uint16_t)(screenRtc.second);

  screen_write_sram(0x009C, dataBuffer, 4);
}

void screen_get_rtc_time(uint8_t *data){
  screen_read_sram(0x10, data, 4);
}

uint32_t screen_get_current_epoch_time(void){
  return (screen_initial_epoch_time + millis()/1000);
}

void screen_save_epoch_time(void){
  screen_initial_epoch_time =  1589411272;
}

bool screen_read_ack(void)
{
  uint8_t ackIndex;
  if(screen_read_frame_full(rxBuffer, &ackIndex))
  {
    if((rxBuffer[3] == 0x82) && (rxBuffer[4] == 0x4F) && (rxBuffer[5] == 0x4B))
      return ACK;
  }

  return false;
}