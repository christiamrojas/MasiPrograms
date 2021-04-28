#ifndef MODEL_H
#define MODEL_H

#include "address_definitions.h"
#include "model_definitions.h"
#include <stdint.h>
#include "view.h"
#include "global_definitions.h"
#include "control_iface.h"
#include "alarm_manager.h"
#include "config.h"
#include "context_mgr.h"
#include "sd.h"
#include "model_vars.h"
#include "sample_mgr.h"
#include "comms_mgr.h"
#include "autodiag_mgr.h"

#define NO_CALC 0
#define IE_CALC 1
#define O2_CALC 2
#define ALL_CALC 3

#define AUTODIAGNOSTIC_ICON 50
#define BACK_ICON 51

#define BATTERY_ICON_REFRESH_RATE 3000

#define THEORETICAL_IE_MIN 0.7
#define THEORETICAL_IE_MAX 6.0

#define COMMS_READ_BUFFER_SIZE 100
#define AVR_CALIBRATION_BUFFER_SIZE 100

typedef enum
{
    INIT_STATE = 0,
    BTN_PRESSED,
    BTN_RELEASED
}encoderBtnValues;

void model_init(void);
void model_init_screen_elements(void);
void model_set_off_values(viewInput* vi);
void model_get_all_elements(viewInput* vi);
int model_handler(modelInput* mo, viewInput* vi);
void model_samples_handler(viewInput *vi);
void model_handle_encoder_cmd(modelInput* mo, viewInput* vi);
void model_struct_handler(modelInput *mo, viewInput *vi);
void model_ctl_iface_handler(viewInput *vi);
void model_enc_btn_handler(viewInput *vi);
void model_handle_btn_cmd(modelInput* mo, viewInput* vi);
bool model_check_if_btn_reset(uint16_t btnId);
void model_handle_device_operation(viewInput *vi);
void model_handle_context(viewInput *vi);
void model_restore_conf_btns(viewInput *vi);
void model_start_context_selection(viewInput *vi, uint8_t targetContext, uint8_t selectionNextContext);
bool model_check_conf_values(uint8_t opMode);
void model_handle_encoder_cmd(modelInput* mo, viewInput* vi);
screen_element model_get_btn_data(uint16_t btnId);
void model_set_vc_cmv_mode(viewInput *vi);
void model_set_pc_cmv_mode(viewInput *vi);
void model_set_pc_csv_mode(viewInput *vi);
void model_set_o2_calibration_operation(void);
void model_set_stop(void);
void model_get_btn_ids(uint8_t varId, uint16_t* ids, uint8_t* idSize);
void model_set_back_icon_change_flag(bool value);
widgetBulk_struct model_update_btns(var_struct var);
widgetBulk_struct model_rst_btn_val(var_struct var, uint16_t lbp);
widget_struct model_update_widget_value(var_struct var, uint16_t reg);
ctl_struct model_check_ctl_iface();
alarm_struct model_set_alarm(uint16_t alarm_id);
void model_log_handler(void);
bool Is_exhalation_time_congruent(void);
bool Is_inhalation_time_congruent(void);
bool Is_PC_congruent(uint8_t opMode);
void model_A1_command_handler(void);
void sd_log_command_sent_to_atmega(uint8_t cmd, uint8_t *dataFrame);
void model_update_widget_array(var_struct var, widgetBulk_struct *wbs, screen_element_type type);
void model_update_widget_array_by_mvs(measuredVars_struct mvs, widgetBulk_struct *wbs);
void model_calc_theoretical_ie(widgetBulk_struct *wbd);
void model_calc_theoretical_o2(widgetBulk_struct *wbd);
void model_handle_power_cycle(viewInput* vi);
void model_set_operation_mode(uint8_t osf, viewInput *vi);
void model_handle_check_battery(viewInput *vi);
void model_send_buzzer_cmd(uint8_t buzzerState);
void model_toggle_pause_state(viewInput *vi);
void model_set_a4_request(void);
void model_handle_comms(viewInput *vi);
void model_autodiag_handler(viewInput *vi);
void model_handle_stop_operation(void);
float model_get_theoretical_ie(uint8_t context);
void model_get_context_state(void);
void model_set_context_state(void);
bool model_check_context_integrity(void);
bool model_comp_navigators(void);
void model_set_widget_to_reset_value(uint16_t varId, float value, viewInput *vi);
void model_restore_reset_values(viewInput* vi, uint16_t *values);
uint8_t model_get_operation_mode(void);
void model_set_min_exp_init_val(void);
void model_store_theoretical_ie(float theoretical_ie);
float model_get_v_theoretical_ie(void);
float model_get_p_theoretical_ie(void);
void model_deselect_current_btn(viewInput *vi);
#endif