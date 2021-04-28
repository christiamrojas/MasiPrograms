#ifndef DATA_MGR_H
#define DATA_MGR_H

#include "Arduino.h"
#include "main_definitions.h"
#include "config.h"

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


typedef struct{
  int year;   //Just the two last digits
  int month;
  int day;
  int hour;
  int minute;
  int second;
}timestamp_struct;

typedef struct{
  uint8_t active_status[ALARM_SIZE] = {0};
  uint32_t active_time[ALARM_SIZE] = {0};
}alarms_struct;


typedef struct{
    float p;
    float f;
    float v;
    float flow_max;
    uint16_t t;
    float PIP;
    float Pmes;
    float Pplateu;
    float FR_measured;
    float PEEP_measured;
    float MVe;
    float VTi;
    float VTe;
    float I;
    float E;
    float HOLD;
    float FR3[3];
    float FPI;               // Flujo pico inspiratorio
    float FMI;               // Flujo medio inspiratorio
    float FMI_acc;           // Acumulador de FMI
    float g_suggested_o2;    // O2 sugerido en graficas
    float c_suggested_o2;    // O2 sugerido en configuracion
    float Im;                // Inspiracion medida de la curva
    float Em;                // Expiracion medida de la curva
    float O2_measured;       // Oxígeno medido
    float voltage_measured;  // Voltaje de la bateria
    float current_measured;  // Corriente medida
    bool AC_on;             // Si estás usando AC o no
    uint8_t over_pressure;
    uint8_t over_flow;
    uint8_t position_alarm;
    float g_calculated_ie;  //IE calculado en graficas
    uint16_t colon;
    float c_static;
    timestamp_struct timestamp;
    uint32_t ts_offset;
    uint8_t alarms[ALARM_SIZE];
    float flow_set_val;
    float weight_set_val;
    float fr_set_val;
    float vt_set_val;
    float trigger_set_val;
    float fio2_set_val;
    float peep_set_val;
    float pc_set_val;
    float ti_set_val;
    float ps_set_val;
    float cycling_set_val;
    float tapnea_set_val;
    float operation_mode_set;
    float c_calculated_ie;  //IE calculado en configuracion
}sample_measured_vars_struct;


void data_mgr_init(void);
void data_mgr_parse_pfv_sample(char *dataBuffer);
void data_mgr_parse_measured_vals_sample(char *dataBuffer);
void pfv_values_handler(char *data_buffer);
void measured_vals_handler(char *data_buffer);
void data_mgr_generate_pfv_json_v2(void);
void add_new_sample_to_samples_to_send(void);
void clean_sample_buffer(void);
void test_send(void);

#endif