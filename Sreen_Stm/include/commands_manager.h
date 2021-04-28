#ifndef COMMANDS_MANAGER_H
#define COMMANDS_MANAGER_H
#include "global_definitions.h"
#include "model_definitions.h"
#include "sd.h"
#include "sample_mgr.h"
#include "context_mgr.h"
#include "autodiag_mgr.h"
#include "comms_mgr.h"

#define FPI_CALIBRATION_FACTOR 2
#define MIN_CYCLES_BEFORE_FR_CHECK 4

#define LEAK_THRESHOLD_PERCENTAGE_VAL 0.4

void cmd_mgr_init(void);
void cmd_mgr_set_test_for_alarm(bool value);
void cmd_mgr_A1(measured_values_struct *measured_values, ctl_struct result,
                bool inspIndicator, bool *p_max_flag, bool *p_min_flag);

void cmd_mgr_A3(measured_values_struct *measured_values, ctl_struct result,
                viewInput *vi, bool *inspIndicator, bool *p_max_flag, 
                bool *p_min_flag);

void cmd_mgr_A4(measured_values_struct *measured_values, ctl_struct result);
widget_struct cmd_mgr_set_widget_struct(measuredVars_struct mvs);
void cmd_mgr_set_widget_bulk_struct(widgetBulk_struct *wbs);
bool cmd_mgr_get_new_battery_data(void);
void cmd_mgr_set_new_battery_data(bool value);
void cmd_mgr_ie_fifo_displace(measured_values_struct *mvs);
float cmd_mgr_avg_ie_values(measured_values_struct *mvs);
float cmd_mgr_get_fr_avg(measured_values_struct *mvs);
bool cmd_mgr_check_cycle_integrity(measured_values_struct *mvs, float currentCycle);
void cmd_mgr_set_stop_flag(bool value);
#endif