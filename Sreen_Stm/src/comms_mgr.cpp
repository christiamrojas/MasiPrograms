#include "model_definitions.h"
#include "comms_mgr.h"

uint32_t syncBase;

void comms_mgr_init(void)
{
    Serial2.begin(COMMS_BAUDRATE);
    pinMode(RF_ENABLE_PIN, OUTPUT);

    digitalWrite(RF_ENABLE_PIN, HIGH);
}

bool comms_mgr_available(void)
{
    if(Serial2.available() > 0)
        return true;
    else   
        return false;
}

uint8_t comms_mgr_read_byte(void)
{
    return Serial2.read();
}

void comms_mgr_write_byte(uint8_t byteValue)
{
    Serial2.write(byteValue);
}

void comms_mgr_write_frame(uint8_t *frame, uint8_t frameSize)
{
    for(int i=0; i<frameSize; i++)
    {
        comms_mgr_write_byte(frame[i]);
    }
}

void comms_mgr_pack(uint8_t cmd, uint8_t *dataFrame, uint8_t dataSize)
{
    uint8_t bufferSize = COMMS_TX_BUFFER_MIN_SIZE + dataSize;
    uint8_t txBuffer[bufferSize];

    txBuffer[0] = COMMS_HEADER1;
    txBuffer[1] = COMMS_HEADER2;

    txBuffer[2] = bufferSize - 3; //Frame Size = size of buffer - headers and frame size
    txBuffer[3] = cmd;

    for(int i=0; i<dataSize; i++)
    {
        txBuffer[i+4] = dataFrame[i];
    }

    comms_mgr_write_frame(txBuffer, bufferSize);
}

bool comms_mgr_read_dataframe(uint8_t *dataBuffer, uint8_t *index)
{
    unsigned long t_start, t_elapsed;
    int timeoutCounter = 0;
    t_start = millis();
    t_elapsed = 0;

    if(comms_mgr_available())
    {
        uint8_t i=0;
        bool endFlag = 0;
        uint8_t packetLen = 0;

        while(endFlag != 1)
        {
            if(t_elapsed >= COMMS_RX_TIMEOUT)
                return false;
            
            if(comms_mgr_available())
            {
                dataBuffer[i] = comms_mgr_read_byte();
                i++;
            }

            if(i==1)
            {
                if(dataBuffer[0] != COMMS_HEADER1)
                    i--;
            }
            else if(i == 2)
            {
                if(dataBuffer[1] != COMMS_HEADER2)
                    i--;
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
        return true;
    }
    return false;
}

void comms_mgr_send_pfv_vals(float p, float f, float v, uint32_t currentMillis)
{   
    uint8_t frameSize = (FLOAT_SIZE * BASIC_VAL_NUM) + TIMESTAMP_SIZE;
    uint8_t dataFrame[frameSize];
    uint8_t *pPtr, *fPtr, *vPtr;

    pPtr = (uint8_t *)&p;
    fPtr = (uint8_t *)&f;
    vPtr = (uint8_t *)&v;

    for(int i=0; i<FLOAT_SIZE; i++)
    {
        dataFrame[i] = *(pPtr + i);
        dataFrame[i+4] = *(fPtr + i);
        dataFrame[i+8] = *(vPtr + i);
    }
        
    dataFrame[(FLOAT_SIZE * BASIC_VAL_NUM)] = (uint8_t)((currentMillis >> 24) & 0xFF);
    dataFrame[(FLOAT_SIZE * BASIC_VAL_NUM) + 1] = (uint8_t)((currentMillis >> 16) & 0xFF);
    dataFrame[(FLOAT_SIZE * BASIC_VAL_NUM) + 2] = (uint8_t)((currentMillis >> 8) & 0xFF);
    dataFrame[(FLOAT_SIZE * BASIC_VAL_NUM) + 3] = (uint8_t)(currentMillis & 0xFF);

    comms_mgr_pack(0x01, dataFrame, frameSize);
}

void comms_mgr_send_measured_vals(float *mva, uint8_t *almsToSend, float *setVars)
{
    // rtc_struct rtcData;

    // uint8_t frameSize = TIMESTAMP_SIZE + (4 * MEASURED_VALS_NUM_TO_SEND);
    uint8_t frameSize = (FLOAT_SIZE * MEASURED_VALS_NUM_TO_SEND) + ALARM_SIZE + (FLOAT_SIZE * SET_VALS_NUM_TO_SEND) + TIMESTAMP_SIZE;
    uint8_t dataFrame[frameSize];
    uint8_t idx = 0;
    uint32_t currentMillis;
    int loop_start, loop_end;

    for(int i=0; i<MEASURED_VALS_NUM_TO_SEND; i++)
    {
        comms_mgr_insert_float(mva[i], dataFrame, idx);
        idx += 4;
    }

    loop_start = (FLOAT_SIZE * MEASURED_VALS_NUM_TO_SEND);
    loop_end = ((FLOAT_SIZE * MEASURED_VALS_NUM_TO_SEND) + ALARM_SIZE);

    for(int i=loop_start; i<loop_end; i++)
    {
        dataFrame[i] = almsToSend[i - (4 * MEASURED_VALS_NUM_TO_SEND)];
    }

    loop_start = ((FLOAT_SIZE * MEASURED_VALS_NUM_TO_SEND) + ALARM_SIZE);
    loop_end = (FLOAT_SIZE * MEASURED_VALS_NUM_TO_SEND) + ALARM_SIZE + (FLOAT_SIZE * SET_VALS_NUM_TO_SEND);
    
    idx = loop_start;

    // uint8_t tempBufferSize = FLOAT_SIZE * SET_VALS_NUM_TO_SEND;
    // uint8_t tempBuffer[tempBufferSize];
    // uint16_t tempIdx = 0;

    // for(int i=0; i<tempBufferSize; i++)
    // {
        
    // }
    
    for(int i=0; i<SET_VALS_NUM_TO_SEND; i++)
    {
        comms_mgr_insert_float(setVars[i], dataFrame, idx);
        idx += 4;
    }

    currentMillis = comms_mgr_get_current_millis();
    
    int millisStartIdx = ((FLOAT_SIZE * MEASURED_VALS_NUM_TO_SEND) + ALARM_SIZE) + (FLOAT_SIZE * SET_VALS_NUM_TO_SEND);

    dataFrame[millisStartIdx] = (uint8_t)((currentMillis >> 24) & 0xFF);
    dataFrame[millisStartIdx + 1] = (uint8_t)((currentMillis >> 16) & 0xFF);
    dataFrame[millisStartIdx + 2] = (uint8_t)((currentMillis >> 8) & 0xFF);
    dataFrame[millisStartIdx + 3] = (uint8_t)(currentMillis & 0xFF);

    comms_mgr_pack(0x02, dataFrame, frameSize);
}

void comms_mgr_insert_float(float value, uint8_t *df, uint16_t dataIdx)
{
    uint8_t *mvsPtr;
    mvsPtr = (uint8_t *)&value;

    for(int i=0; i<4; i++)
        df[dataIdx + i] = *(mvsPtr + i);
}

void comms_mgr_send_alarm(uint8_t alarmCode)
{
    //  rtc_struct rtcData;

    // uint8_t frameSize = TIMESTAMP_SIZE + 1;
    uint8_t frameSize = 1;
    uint8_t dataFrame[frameSize];

    dataFrame[0] = alarmCode;

    // #ifdef RTC_FLAG
    //     rtc_get_timestamp(&rtcData);
    // #else 
    //     rtc_get_timestamp_aux(&rtcData);
    // #endif

    // dataFrame[1] = (uint8_t)rtcData.year;
    // dataFrame[2] = (uint8_t)rtcData.month;
    // dataFrame[3] = (uint8_t)rtcData.day;
    // dataFrame[4] = (uint8_t)rtcData.hour;
    // dataFrame[5] = (uint8_t)rtcData.minute;
    // dataFrame[6] = (uint8_t)rtcData.second;

    comms_mgr_pack(0x03, dataFrame, frameSize);
}

void comms_mgr_enable_rf(void)
{
    digitalWrite(RF_ENABLE_PIN, HIGH);
}

void comms_mgr_disable_rf(void)
{
    digitalWrite(RF_ENABLE_PIN, LOW);
}

void comms_mgr_sync_time(void)
{
    uint8_t tokenData[1];
    tokenData[0] = 1;

    //comms_mgr_pack(0x04, tokenData, 0);
    syncBase = millis();
}

uint32_t comms_mgr_get_current_millis(void)
{
    uint32_t currentMillis;

    currentMillis = millis();
    
    currentMillis = currentMillis - syncBase;

    return currentMillis;
}
