#ifndef SCREEN_DRIVER_H
#define SCREEN_DRIVER_H

#include <stdint.h>
#include "Arduino.h"
#include "config.h"
#include "global_definitions.h"

#define BAUDRATE 115200
#define RXBUFFER_MAXLEN 32
#define TIMEOUT_MS 100

#define HEADER1 0x5A
#define HEADER2 0xA5

#define FIXED_PAYLOAD_SIZE 3

#define ACK 0x4F4B

#define READ_SRAM_TIMEOUT_MS 200

void screen_init(void);
long screen_get_time();
void screen_write_byte(uint8_t dataByte);
void screen_write_frame(uint8_t *dataFrame, uint8_t dataLen);
bool screen_available(void);
uint8_t screen_read(void);
bool screen_read_byte(uint8_t *dataByte);
bool screen_read_frame(uint8_t frameLen);
bool screen_read_frame_full(uint8_t* dataBuffer, uint8_t* index);
void screen_write_reg(uint8_t reg, uint8_t *dataFrame, uint8_t frameLen);
int screen_read_reg(uint8_t reg, uint8_t *dataFrame, uint8_t frameLen);
void screen_write_sram(uint16_t reg, uint16_t *data, uint8_t dataLen);
uint8_t screen_read_sram(uint16_t reg, uint8_t *data, uint8_t txDataLen);
void screen_write_graph(uint8_t ch_mode, uint16_t *data, uint8_t dataLen);
void view_write_graph_single(uint16_t datapoint, uint8_t channel);
uint16_t screen_read_button(void);
void screen_beep_buzzer(uint8_t times_10ms);
void screen_set_rtc_time(void);
void screen_set_rtc(screenRtc_struct screenRtc);
void screen_get_rtc_time(uint8_t *data);
uint32_t screen_get_current_epoch_time(void);
void screen_save_epoch_time(void);
bool screen_read_ack(void);
void screen_flush_rx_buffer(void);
#endif
