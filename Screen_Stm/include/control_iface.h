#ifndef CONTROL_IFACE_H
#define CONTROL_IFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "Arduino.h"
#include "config.h"

#define BAUDRATE 115200
#define RXBUFFER_MAXLEN 32
#define TXBUFFER_MAXLEN 32

#define RXTIMEOUT 100

#define HEADER1 0x5A
#define HEADER2 0xA5

void control_iface_init(void);
void control_iface_write_frame(uint8_t *dataFrame, uint8_t frameLen);
void control_iface_write_byte(uint8_t dataByte);
bool control_iface_available();
uint8_t control_iface_read();
bool control_iface_get(uint8_t *dataFrame);
void control_iface_pack(uint8_t cmd, uint8_t* dataFrame);
bool control_iface_check_data(uint8_t* dataBuffer, uint8_t* index);
uint8_t control_iface_get_cmd(uint8_t* dataBuffer, uint8_t bufferIndex);

#endif