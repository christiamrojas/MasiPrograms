#include <Arduino.h>
#include "utils.h"
#include <RTClock.h>
#include "screen_driver.h"
#include "rtc.h"

//tm_t new_time;
auxRtc_struct auxRtc;

unsigned long lastMillis;
unsigned long currentMillis;

void rtc_set_time(void){
    //Jalamos la hora de la pantalla
    uint8_t timestamp[16] = {0x00};
    screen_get_rtc_time(timestamp);

    #ifdef RTC_FLAG
    timestamp[3] = timestamp[3] + 2000 - 1970; //Offset from 1970
    new_time.year = timestamp[3];
    new_time.month = timestamp[4];
    new_time.day = timestamp[5];
    new_time.hour = timestamp[7];
    new_time.minute = timestamp[8];
    new_time.second = timestamp[9];
    rt.setTime(new_time);
    #else
    auxRtc.year = timestamp[3]; //Offset desde el 2000
    auxRtc.month = timestamp[4];
    auxRtc.day = timestamp[5];
    auxRtc.hour = timestamp[7];
    auxRtc.minute = timestamp[8];
    auxRtc.second = timestamp[9];
    auxRtc.millisecs = 0;
    #endif


}

void rtc_get_timestamp_aux(rtc_struct *rtc_time)
{
    rtc_time->year = auxRtc.year;
    rtc_time->month = auxRtc.month;
    rtc_time->day = auxRtc.day;
    rtc_time->hour = auxRtc.hour;
    rtc_time->minute = auxRtc.minute;
    rtc_time->second = auxRtc.second;
}

void rtc_update_aux_rtc(void)
{
    currentMillis = millis();
    long millisDiff = currentMillis - lastMillis;
    if(millisDiff > 0)
    {
        auxRtc.millisecs += millisDiff;
        lastMillis = currentMillis;
        if(auxRtc.millisecs >= 1000)
        {
            auxRtc.second++;
            auxRtc.millisecs = 0;
        }
    }
    if(auxRtc.second == 60)
    {
        auxRtc.minute++;
        auxRtc.second = 0;
    }
    if(auxRtc.minute == 60)
    {
        auxRtc.hour++;
        auxRtc.minute = 0;
    }
    if(auxRtc.hour == 24)
    {
        auxRtc.day++;
        auxRtc.hour = 0;
    }

    //Verificamos si tenemos que cambiar de mes
    if((auxRtc.month == 1) || (auxRtc.month == 3) || (auxRtc.month == 5) || (auxRtc.month == 7) || (auxRtc.month == 8) || (auxRtc.month == 10) || (auxRtc.month == 12))
    {
        if(auxRtc.day == 32)
        {
            auxRtc.month++;
            auxRtc.day = 1;
        }
    }
    else if((auxRtc.month == 2))
    {
        if(auxRtc.day == 29)
        {
            auxRtc.month++;
            auxRtc.day = 1;
        }
    }
    else
    {
        if(auxRtc.day == 31)
        {
            auxRtc.month++;
            auxRtc.day = 1;
        }
    }
    if(auxRtc.month == 13)
    {
        auxRtc.year++;
        auxRtc.month = 1;
    }
}

void rtc_convert_epoch_to_struct(uint32_t epoch, viewInput *vi)
{
    vi->screenRtc_data.year = year((time_t)epoch);
    vi->screenRtc_data.month = (uint8_t)month((time_t)epoch);
    vi->screenRtc_data.day = (uint8_t)day((time_t)epoch);
    vi->screenRtc_data.hour = (uint8_t)hour((time_t)epoch);
    vi->screenRtc_data.minute = (uint8_t)minute((time_t)epoch);
    vi->screenRtc_data.second = (uint8_t)second((time_t)epoch);
    vi->screenRtc_data.active = true;
}