#ifndef MODEL_DEFINITIONS_H
#define MODEL_DEFINITIONS_H

#include "model.h"
#include "config.h"
#include "model_vars.h"

#define NOVAR 0
#define FLOW 1
#define WEIGHT 2
#define FR 3
#define VT 4
#define TRIGGER 5
#define FIO2 6
#define PEEP 7
#define PC 8
#define TI 9
#define PS 10
#define CYCLING 11
#define TAPNEA 12
#define ALARM_PMAX 13
#define ALARM_PMIN 14
#define ALARM_VT_MAX  15
#define ALARM_VT_MIN 16
#define ALARM_FRMAX 17
#define ALARM_PEEP_DELTA 18
#define TIVC 19
#define G_SUGGESTED_O2 20
#define ALARM_FIO2_DELTA 21

#define MINIMUM_INHALATION_TIME 0.5 //Units: segs
#define MINIMUM_EXHALATION_TIME 1   //Units: segs
#define PC_PEEP_DELTA           15

#define HOLD_TIME 30        //15 decimas de segundo

#define EXCEPTIONS_NUM 2

#define RPM_FIFO_SIZE 3
#define IE_FIFO_SIZE 3

typedef struct{
    float p;
    float f;
    float v;
    float flow_max;
    uint16_t t;
    float PIP;
    float Pmes;
    float FR_measured;
    float PEEP_measured;
    float MVe;
    float VTi;
    float VTe;
    float I;
    float E;
    float IE_FIFO[IE_FIFO_SIZE];
    float HOLD;
    float FR3[RPM_FIFO_SIZE];
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
    float c_calculated_ie;  //IE calculado en configuracion
    uint16_t colon;
    float resistance;
    bool apneaTrigger;
    bool sensorDisconnect;
    bool disconnection;
    bool obstruction;
}measured_values_struct;

screen_element model_def_get_element_by_idx(int i);
var_struct model_def_get_var(uint16_t varId);
void model_def_set_var(var_struct var);
var_struct model_def_get_var_by_idx(int idx);
bool model_def_store_measuredval_value(float value, int id);
measuredVars_struct model_def_get_measuredvar_by_idx(int idx);
float model_def_get_var_real_val(var_struct var);
measuredVars_struct model_def_get_measured_var_by_id(uint16_t varId);
screen_element model_def_get_scr_element_by_var(uint16_t varId, screen_element_type type);
bool model_def_check_exceptions(uint16_t checkId);
screen_element model_def_get_scr_element_by_btn_val(uint16_t btnValue);
void model_def_set_scr_element_by_btn_val(uint16_t btnValue, screen_element se);
void model_def_set_stop_flag(bool value);
bool model_def_get_stop_flag(void);
void model_def_backup_vars(void);
void model_def_print_backup_vals(void);
void model_def_restore_vars_from_backup(void);
void model_def_print_vars(void);
#endif