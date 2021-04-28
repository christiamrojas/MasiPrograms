#ifndef SIGFOX_H
#define SIGFOX_H

#include "config.h"

#define SIGFOX_ENABLE PA11

void sigfox_init(void);
void sigfox_change_to_region_4(void);
void sigfox_read_info(void);
void sigfox_send_AT_command(char* comandoAT);
void sigfox_send_message(String buf_tx);

#endif