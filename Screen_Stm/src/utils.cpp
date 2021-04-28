#include <math.h>
#include <Arduino.h>
#include "rtc.h"
#include "screen_driver.h"
#include "utils.h"

uint8_t bcd_to_dec(uint8_t bcd_num){
    uint8_t msb_nible = (bcd_num & 0xF0) >> 4;
    uint8_t lsb_nible = (bcd_num & 0x0F);
    return (msb_nible*10 + lsb_nible);
}

void int_to_str(int num, int num_digits, char *buf){
    int digit = 0;

    for(int i=num_digits-1; i >=0; i--){
        digit = num % 10;
        buf[i] = digit + '0';
        num = num / 10;
    }
}

void float_to_str(float num, char *buf){
    int num_digits = 0;
    int digit = 0;
    int end_loop_idx = 0;
    bool is_negative = false;
    const int min_digits = 3;
    const int max_digits = 6;

    int new_num = (int)(num*10.0);

    if(new_num < 0){
        is_negative = true;
        new_num *= -1;
        end_loop_idx = 1;
        num_digits += 1;
    }

    if(new_num == 0)
        num_digits += min_digits;
    else
        num_digits += log10(new_num) + 2;

    if(num_digits < min_digits)
        num_digits = min_digits;
    else if(num_digits > max_digits)    
        num_digits = max_digits;

    for(int i=num_digits-1; i >= end_loop_idx; i--){
        if(i == (num_digits-2)){
            buf[i] = '.';
            continue;
        }
        digit = new_num % 10;
        buf[i] = digit + '0';
        new_num = new_num / 10;
    }

    if(is_negative)
        buf[0] = '-';

}

void assign_timestamp_to_filename(rtc_struct *rtc_time, char *filename){
    char str_temp[6] = {'\0'};

    memset(str_temp, '\0', sizeof(str_temp));
    int_to_str(rtc_time->month, 2, str_temp);
    strcat(filename, str_temp);
    strcat(filename, "-");
    
    memset(str_temp, '\0', sizeof(str_temp));
    int_to_str(rtc_time->day, 2, str_temp);
    strcat(filename, str_temp);
    strcat(filename, " ");

    memset(str_temp, '\0', sizeof(str_temp));
    int_to_str(rtc_time->hour, 2, str_temp);
    strcat(filename, str_temp);
    strcat(filename, ".log");
}

void assign_screen_timestamp_to_filename(char *filename){
    uint8_t timestamp[16] = {0x00};
    screen_get_rtc_time(timestamp);

    for(uint8_t idx=0; idx < sizeof(timestamp); idx++)
        timestamp[idx] = bcd_to_dec(timestamp[idx]);

    sprintf(filename, "%.2d-%.2d-%.2d.log", timestamp[0], timestamp[1], timestamp[2]);
}

void add_screen_timestamp_to_log_string(char *buf){
    uint8_t timestamp[16] = {0x00};
    screen_get_rtc_time(timestamp);

    for(uint8_t idx=0; idx < sizeof(timestamp); idx++)
        timestamp[idx] = bcd_to_dec(timestamp[idx]);

    sprintf(buf, "%d-%d-%d %d:%d:%d", timestamp[0], timestamp[1], timestamp[2], timestamp[4],
                                timestamp[5], timestamp[6]);
}

void add_timestamp_to_log_string(rtc_struct *rtc_time, char *buf){
    char str_temp[6] = {'\0'};

    memset(str_temp, '\0', sizeof(str_temp));
    int_to_str(rtc_time->hour, 2, str_temp);
    strcat(buf, " ");
    strcat(buf, str_temp);
    strcat(buf, ":");

    memset(str_temp, '\0', sizeof(str_temp));
    int_to_str(rtc_time->minute, 2, str_temp);
    strcat(buf, str_temp);
    strcat(buf, ":");
    
    memset(str_temp, '\0', sizeof(str_temp));
    int_to_str(rtc_time->second, 2, str_temp);
    strcat(buf, str_temp);
}

void generate_untouched_file_names(rtc_struct *rtc_time, char filenames_not_to_be_removed[][15]){
    rtc_struct iterable_rtc;
    int idx_filename = 0;
    int current_day = rtc_time->day;
    int current_month = rtc_time->month;
    int current_hour = rtc_time->hour;
    iterable_rtc.month = current_month;
    iterable_rtc.day = current_day;

    //Generamos los nombres de los archivos para el día actual
    for(int idx_hour = 0; idx_hour <= current_hour; idx_hour++){
        iterable_rtc.hour = idx_hour;
        assign_timestamp_to_filename(&iterable_rtc, filenames_not_to_be_removed[idx_filename]);
        idx_filename++;
    }
    
    //Generamos los nombres de los archivos para el día anterior
    current_day -= 1;

    //Chequeamos si tenemos que volver al último día del mes anterior
    if(current_day == 0){
        current_month -= 1;

        if(current_month == 0){ //Chequeamos si volvemos a diciembre
            current_month = 12;
        }

        if((current_month ==1) || (current_month ==3) || (current_month ==5) || (current_month ==7) || (current_month ==8) || (current_month ==10) || (current_month ==12)){
            current_day = 31;
        }
        else if(current_month == 2){
            current_day = 28;
        }
        else{
            current_day = 30;
        }
    }

    iterable_rtc.month = current_month;
    iterable_rtc.day = current_day;
    for(int idx_hour = 0; idx_hour <= 23; idx_hour++){
        iterable_rtc.hour = idx_hour;
        assign_timestamp_to_filename(&iterable_rtc, filenames_not_to_be_removed[idx_filename]);
        idx_filename++;
    }

    /*
    Serial.println("Archivos que NO se van a borrar:");
    for(int idx = 0; idx < idx_filename; idx++){
        Serial.println(filenames_not_to_be_removed[idx]);
    }
    */
}

bool Is_filename_in_array(char *filename, char filenames_not_to_be_removed[][15]){

    for(int idx = 0; idx < 50; idx++){
        if(strcmp(filename, filenames_not_to_be_removed[idx]) == 0)
            return true;
    }

    return false;
}