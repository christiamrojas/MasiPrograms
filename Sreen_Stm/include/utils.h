#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include "rtc.h"


uint8_t bcd_to_dec(uint8_t bcd_num);
void int_to_str(int num, int num_digits, char *buf);
void float_to_str(float num, char *buf);
void add_timestamp_to_log_string(rtc_struct *rtc_time, char *buf);
void assign_timestamp_to_filename(rtc_struct *rtc_time, char *filename);
void assign_screen_timestamp_to_filename(char *filename);
void add_screen_timestamp_to_log_string(char *buf);
void generate_untouched_file_names(rtc_struct *rtc_time, char filenames_not_to_be_removed[][15]);
bool Is_filename_in_array(char *filename, char filenames_not_to_be_removed[][15]);
#endif