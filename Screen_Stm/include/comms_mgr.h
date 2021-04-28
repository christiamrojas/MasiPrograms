#ifndef COMMS_MGR_H
#define COMMS_MGR_H

#include "Arduino.h"
#include "rtc.h"
#include "model_definitions.h"

#define COMMS_BAUDRATE 115200

#define RF_ENABLE_PIN PA1

#define COMMS_HEADER1 0x5A
#define COMMS_HEADER2 0xA5

#define COMMS_RX_TIMEOUT 100
#define COMMS_TX_BUFFER_MIN_SIZE 4

#define BASIC_VAL_NUM 3
#define TIMESTAMP_SIZE 4
#define FLOAT_SIZE 4
#define MEASURED_VALS_NUM_TO_SEND 13
#define SET_VALS_NUM_TO_SEND 14

void comms_mgr_init(void);
bool comms_mgr_available(void);
uint8_t comms_mgr_read_byte(void);
void comms_mgr_write_byte(uint8_t byteValue);
void comms_mgr_pack(uint8_t cmd, uint8_t *dataFrame, uint8_t dataSize);
bool comms_mgr_read_dataframe(uint8_t *dataBuffer, uint8_t *index);
void comms_mgr_write_frame(uint8_t *frame, uint8_t frameSize);
void comms_mgr_send_pfv_vals(float p, float f, float v, uint32_t currentMillis);
void comms_mgr_send_measured_vals(float *mva, uint8_t *almsToSend, float *setVars);
void comms_mgr_insert_float(float value, uint8_t *df, uint16_t dataIdx);
void comms_mgr_send_alarm(uint8_t alarmCode);
void comms_mgr_enable_rf(void);
void comms_mgr_disable_rf(void);
void comms_mgr_sync_time(void);
uint32_t comms_mgr_get_current_millis(void);
#endif