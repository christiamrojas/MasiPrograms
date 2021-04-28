#ifndef SERIAL_MGR_H
#define SERIAL_MGR_H

#include <Arduino.h>

#define COMMS_HEADER1 0x5A
#define COMMS_HEADER2 0xA5

#define COMMS_RX_TIMEOUT 100
#define COMMS_TX_BUFFER_MIN_SIZE 4

static const long BAUDRATE = 115200;
static const int RX_BUFFER_SIZE = 500;
static const int RX_TIMEOUT = 100;

static const uint8_t HEADER1 = 0x5A;
static const uint8_t HEADER2 = 0xA5;

void serial_mgr_init(void);
bool serial_mgr_available(void);
uint8_t serial_mgr_read_byte(void);
bool serial_mgr_get_frame(uint8_t *serial_rx_buffer, uint16_t *serial_rx_buffer_idx);
void serial_mgr_write_byte(uint8_t byteValue);
void serial_mgr_write_frame(uint8_t *frame, uint8_t frameSize);
void serial_mgr_pack(uint8_t cmd, uint8_t *dataFrame, uint8_t dataSize);


#endif