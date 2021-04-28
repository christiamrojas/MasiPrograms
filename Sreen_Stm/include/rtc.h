#ifndef RTC_H
#define RTC_H
#include <RTClock.h>
#include "config.h"
#include <time.h>
#include <stdio.h>
#include "global_definitions.h"

typedef struct{
    int year;   //Just the two last digits
    int month;
    int day;
    int hour;
    int minute;
    int second;
}rtc_struct;

typedef struct
{
    int year;   //Just the two last digits
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecs;
}auxRtc_struct;

//extern RTClock rt;

void rtc_set_time(void);
//void rtc_get_timestamp(rtc_struct *rtc_time);
void rtc_get_timestamp_aux(rtc_struct *rtc_time);
void rtc_update_aux_rtc(void);
void rtc_convert_epoch_to_struct(uint32_t epoch, viewInput *vi);
#endif