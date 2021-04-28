#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <stdint.h>
#include "Arduino.h"
#include "global_definitions.h"
#include "config.h"

#define ALARM_NUM 17

#define ALARM_VTMAX_ICON_VAL  11
#define ALARM_VTMIN_ICON_VAL  12
#define ALARM_PMAX_ICON_VAL   13
#define ALARM_PMIN_ICON_VAL   14   
#define ALARM_FRMAX_ICON_VAL  15
#define ALARM_TAPNEA_ICON_VAL 16
#define ALARM_PEEP_MAX_ICON_VAL 17
#define ALARM_FIO2_ICON_VAL     18
#define ALARM_POSITION_ICON_VAL 19
#define ALARM_OVERPRESSURE_ICON_VAL   20
#define ALARM_OVERFLOW_ICON_VAL       21
#define ALARM_DISCONNECTION_ICON_VAL  22
#define ALARM_LEAK_ICON_VAL           23
#define ALARM_OCLUSION_ICON_VAL       24
#define ALARM_AC_STATUS_ICON_VAL      25
#define ALARM_BATTERY_LEVEL_ICON_VAL  26

#define NO_ALARM                      0

#define ALM_MAX_TIME          5000
#define ALM_BLINK_PERIOD      1000
#define ALM_ARRAY_START       -1

#define BUZZER_TIME           50
#define BUZZER_TIME_WINDOW    BUZZER_TIME * 10

#define BATTERY_ALARM_THRESHOLD   24.6  //Units: Volts
#define CURRENT_ALARM_FLAG        -1
#define DISCONNECTION_ALARM_THRESHOLD 2 //Units: cmH2O
#define LEAK_ALARM_THRESHOLD_FIXED 120        //Units: ml
#define OCLUSION_FLOW_THRESHOLD 20      //Units: ml/min
#define OCLUSION_RESISTANCE_THRESHOLD 1000

#define CYCLE_COUNTER_THRESHOLD 4

#define SILENT_ALM_TIMEOUT 300000 // Units: ms

typedef enum
{
  ALARM_VTMAX_IDX = 0,
  ALARM_VTMIN_IDX,
  ALARM_PMAX_IDX,
  ALARM_PMIN_IDX,
  ALARM_FRMAX_IDX,
  ALARM_TAPNEA_IDX,
  ALARM_PEEP_MAX_IDX,
  ALARM_FIO2_IDX,
  ALARM_POSITION_IDX,
  ALARM_OVERPRESSURE_IDX,
  ALARM_OVERFLOW_IDX,
  ALARM_DISCONNECTION_IDX,
  ALARM_LEAK_IDX,
  ALARM_OCLUSION_IDX,
  ALARM_AC_STATUS_IDX,
  ALARM_BATTERY_LEVEL_IDX,
  ALARM_SIZE,
}alm_positions;

typedef struct
{
  bool active;
  bool current;
  uint8_t alarmId;
  uint16_t alarmCode;
}alarmInstance_struct;


typedef struct
{
  bool  active;
  bool previousState;
  long timerStart;
  long timerCurrent;
  long blinkTimerStart;
  long blinkTimerCurrent;
  long buzzerTimerStart;
  long buzzerTimerCurrent;
  bool blinkFlag;
  int currentId;
  uint8_t numActive;
  uint8_t maxSize;
  uint8_t currentActive;
  bool activeAlarms[ALARM_NUM];
  alarmInstance_struct alarms[ALARM_NUM];
}alarms_struct;

typedef enum{
  ALM_BUZZER_DISABLED = 0,
  ALM_BUZZER_ENABLED
}alm_buzzer_modes;

void alm_mgr_init(void);
void alm_mgr_chck_alms(alarm_struct *as, buzzer_struct *bs, ledCtl_struct *lcs);
int alm_mgr_get_next_active_alm(int startIndex);
void alm_mgr_set_alarm(uint16_t alarm_id, alarm_struct *as);
void alm_mgr_activate(uint8_t alarmIndex);
void alm_mgr_deactivate(alarm_struct *as, buzzer_struct *bs, int alarm_id);
void alm_mgr_set_silent_alm(bool value);
bool alm_mgr_get_silent_alm(void);
void alm_mgr_reset_cycle_counter(void);
void alm_mgr_inc_cycle_counter(void);
uint8_t alm_mgr_get_idx_by_code(uint8_t alarmCode);
bool alm_mgr_get_buzzer_flag(void);
bool alm_mgr_get_active_status_by_code(uint8_t alarmCode);
void alm_mgr_set_record_alarm_flag(bool value);
uint8_t alm_mgr_get_cycle_counter(void);
bool alm_mgr_check_cycle_counter(uint8_t num_cycles);
bool alm_mgr_get_stability_flag(void);
void alm_mgr_start_silent_timer(void);
void alm_mgr_set_alarm_cycle_detector_flag(bool value);
uint8_t alm_mgr_get_num_alarms_active(void);
void alm_mgr_deactivate_all_alarms(alarm_struct *as, buzzer_struct *bs);
#endif