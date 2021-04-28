#ifndef GLOBAL_DEFINITIONS_H
#define GLOBAL_DEFINITIONS_H

#include <stdint.h>
#include "config.h"

#define VIEW 0
#define MODEL 1

#define BASE_ELEMENT_NUM 69

#define ELEMENT_NUM BASE_ELEMENT_NUM + 3

#define SCREEN_DATA_BUFFER_SIZE 1
#define SCREEN_DATA_BUFFER_MAX 8

typedef enum
{
    RED = 0,
    GREEN,
    BLUE,
}ledColors;

//############################################
//#############viewInput Structures###########
//############################################
typedef struct{
    bool active;
    uint8_t cmd;
    float p;
    float f;
    float v;
    uint8_t stage;
    bool start;
    bool pfv_flag;
    uint8_t o2;
    float volt;
    float i;
    uint8_t ac;
    uint8_t over_pressure;
    uint8_t over_flow;
    uint8_t position_alarm;
    bool apneaTrigger;
    bool sensorDisconnect;
    bool disconnection;
    bool obstruction;
    uint8_t operationMode;
    uint8_t triggeredInsp;
}ctl_struct;

typedef struct{
  bool active;
  uint16_t lastButtonId;
  uint16_t lastButtonVal;
  uint16_t currentButtonId;
  uint16_t currentButtonVal;
}btnCtl_struct;

typedef struct{
  bool active;
  uint16_t btnId;
  uint16_t btnVal;
}btnIconCtl_struct;

typedef struct
{
  bool active;
  uint16_t reg;
  float val;
  uint8_t decimal;
}widget_struct;


typedef struct 
{
  bool active;
  widget_struct widget_array[ELEMENT_NUM];
  uint8_t index;
}widgetBulk_struct;

typedef struct
{
  uint16_t btnId;
  uint16_t btnVal;
}singleBtn_struct;


typedef struct
{
  bool active;
  singleBtn_struct btnArray[ELEMENT_NUM];
  uint16_t index;
}multiBtnCtl_struct;

typedef struct
{
  bool active;
  uint16_t alarm_id;
}alarm_struct;

typedef struct
{
  bool active;
  uint8_t buzzTime;
}buzzer_struct;

typedef struct
{
  bool active;
  uint16_t screenId;
} screen_struct;

typedef struct 
{
  bool active;
  uint16_t iconVal;
} batteryIcon_struct;

typedef struct
{
  bool active;
}clearScreen_struct;

typedef struct 
{
  bool active;
  uint8_t ledColor;
}ledCtl_struct;


typedef struct
{
  bool active;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
}screenRtc_struct;

//############################################
//############modelInput Structures###########
//############################################

typedef struct{
  bool active;
  uint16_t buttonId;
}button_struct;

typedef struct{
  bool active;
  bool buttonStatus;
  uint8_t encValue;
}encInput_struct;

//############################################
//##############Base Structures###############
//############################################

typedef struct {
  ctl_struct ctl_data;
  btnCtl_struct btnCtl_data;
  widget_struct widget_data;
  widgetBulk_struct widgetBulk_data;
  alarm_struct alarm_data;
  buzzer_struct buzzer_data;
  screen_struct screen_data;
  multiBtnCtl_struct multiBtnCtl_data;
  btnIconCtl_struct btnIconCtl_data;
  batteryIcon_struct batteryIcon_data;
  clearScreen_struct clearScreen_data;
  ledCtl_struct ledCtl_data;
  screenRtc_struct screenRtc_data;
}viewInput;

typedef struct {
  button_struct button_data;
  encInput_struct encInput_data;
}modelInput;


#endif
