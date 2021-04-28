#ifndef MODEL_VARS_H
#define MODEL_VARS_H

#include <stdint.h>

#define MAX_SAMPLE_SIZE 8

#define VAR_NUM 22
#define MEASURED_VARS 20

typedef enum{
  SET = 1,
  CONFIG
}screen_element_type;


typedef struct{
  uint16_t buttonValue;
  uint16_t buttonSelect;
  uint16_t onValue;
  uint16_t offValue;
  uint8_t varId;
  screen_element_type type;
}screen_element;


typedef struct{
  uint8_t varId;
  float varValue;
  float defaultVal;
  float minVal;
  float maxVal;
  float step;
  uint8_t decimal;
}var_struct;

typedef struct
{
  uint8_t varId;
  float varValue;
}varsBackup_struct;

typedef struct 
{
  uint16_t buttonValue;
  uint16_t buttonSelect;
  float measuredVal;
  uint8_t decimal;
}measuredVars_struct;

typedef struct 
{
  float p;
  float f;
  float v;
  uint32_t millisSinceStart;
}sample_struct;

typedef struct
{
  int index;
  int samplesToWrite;
  bool writeFlag;
  sample_struct samples[MAX_SAMPLE_SIZE];
}samples_struct;

#endif