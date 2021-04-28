#include "serial_mgr.h"
#include <SoftwareSerial.h>

//SoftwareSerial sw_serial(D2, D4);      // RX, TX

void serial_mgr_init(void)
{
    //sw_serial.begin(BAUDRATE);
    Serial.begin(BAUDRATE);
    //sw_serial.flush();
}

bool serial_mgr_available(void)
{
    if(Serial.available() > 0)   //sw_serial
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

uint8_t serial_mgr_read_byte(void)
{
    return Serial.read();    //sw_serial
}

bool serial_mgr_get_frame(uint8_t *serial_rx_buffer, uint16_t *serial_rx_buffer_idx)
{
    long t_start, t_elapsed;

    t_start = millis();
    t_elapsed = 0;

    if(serial_mgr_available())
    {
        uint8_t i=0;
        bool endFlag = 0;
        uint8_t frameLen = 0;

        while(endFlag != 1)
        {
            if(t_elapsed >= RX_TIMEOUT)
            {
                return false;
            }

            if(serial_mgr_available())
            {
                serial_rx_buffer[i] = serial_mgr_read_byte();
                i++;
            }

            if(i == 1)
            {
                if(serial_rx_buffer[0] != HEADER1)
                {
                    i--;
                }
            }
            else if(i == 2)
            {
                if(serial_rx_buffer[1] != HEADER2)
                {
                    i--;
                }
            }
            else if(i == 3)
            {
                frameLen = serial_rx_buffer[2];
            }
            else if(frameLen != 0 && i >= (frameLen + 3))
            {
                endFlag = 1;
            }

            t_elapsed = millis() - t_start;
        }

        *serial_rx_buffer_idx = i;

        return true;
    }

    return false;
}

void serial_mgr_write_byte(uint8_t byteValue)
{
    Serial.write(byteValue);
}

void serial_mgr_write_frame(uint8_t *frame, uint8_t frameSize)
{
    for(int i=0; i<frameSize; i++)
    {
        serial_mgr_write_byte(frame[i]);
    }
}

void serial_mgr_pack(uint8_t cmd, uint8_t *dataFrame, uint8_t dataSize)
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

    serial_mgr_write_frame(txBuffer, bufferSize);
}