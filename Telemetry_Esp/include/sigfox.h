#ifndef SIGFOX_H
#define SIGFOX_H
#include <Arduino.h>
#include "config.h"
#include "data_mgr.h"

#define SIGFOX_ENABLE 13
#define SIGFOX_ALARM_ACTIVE_TIME    60000 //Units: ms
extern char ID[51];
extern char PAC[51];


void sigfox_init(void);
void sigfox_change_to_region_4(void);
void sigfox_read_info(void);
void sigfox_send_AT_command(char* comandoAT);
void sigfox_send_message(String buf_tx);
void sigfox_send_active_alarms(sample_measured_vars_struct *sample_data, alarms_struct *active_alms_data);

#endif