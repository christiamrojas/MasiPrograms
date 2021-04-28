#ifndef SAMPLE_MGR_H
#define SAMPLE_MGR_H

#include "Arduino.h"
#include "model_vars.h"
#include "sd.h"
#include "comms_mgr.h"

#define MAX_SAMPLE_SIZE 8   
#define F_BUFFER_SIZE 8
#define AVG_BUFFER_SIZE 4

#define F_THRESHOLD_INSP 0.5
#define F_THRESHOLD_EXP  -0.5

#define F_THRESHOLD_INSP_FACTOR 0.4
#define F_THRESHOLD_EXP_FACTOR 0.5

#define F_THRESHOLD_EXP_MAX_FACTOR 0.5

#define MILLIS_FLOAT 1000.0

// #define ERROR_CONST 0.06
#define ERROR_CONST 0

#define TI_ACTIVE true
#define TI_INACTIVE false

#define DEV_RUNNING true
#define DEV_STOPPED false

#define NUM_SAMPLES 2
#define MAX_SAMPLES_TO_SEND 2

#define MOD_SAMPLES_TO_SEND 4

#define FIFO_SIZE 60

#define PFV_COMMS_COUNTER_THRESHOLD 4

#define TIME_BETWEEN_SAMPLES_MS 10


typedef struct
{
    float samples[F_BUFFER_SIZE];
    float avgSamples[AVG_BUFFER_SIZE];
    unsigned long sampleCounter;
    float fmax;
}fsamples_struct;

typedef enum
{   
    NO_THRESHOLD_UPDATE = 0,
    AVG,
    MAX
}thresholdTypes;

void sample_mgr_init(void);
void sample_mgr_store_sample(float p, float f, float v);
bool sample_mgr_get_write_flag(void);
int sample_mgr_get_write_index(void);
void sample_mgr_get_samples_by_idx(int idx, float *p, float *f, float *v);
void sample_mgr_store_fsample(float f);
float sample_mgr_get_f_avg(void);
void sample_mgr_store_f_avg(float fAvg);
void sample_mgr_write_f_val(float f, bool insp);
void sample_mgr_check_max_f(float f);
float sample_mgr_get_max_f(void);
bool sample_mgr_check_f_threshold_insp(uint8_t filter);
bool sample_mgr_check_f_threshold_exp(uint8_t filter);
unsigned long sample_mgr_get_time_ref(void);
void sample_mgr_store_cycle_start(unsigned long timeRef);
void sample_mgr_store_cycle_end(unsigned long timeRef);
void sample_mgr_store_ti_start(unsigned long timeRef);
void sample_mgr_store_ti_end(unsigned long timeRef);
void sample_mgr_store_alt_ti_end(unsigned long timeRef);
float sample_mgr_calc_cycle_len(void);
float sample_mgr_calc_ti_len(bool altEnd);
void sample_mgr_set_ti_state(bool state);
bool sample_mgr_get_ti_state(void);
void sample_mgr_calc_ti(bool altEnd);
void sample_mgr_calc_cycle(void);
float sample_mgr_get_ti_len(void);
float sample_mgr_get_cycle_len(void);
void sample_mgr_check_ti(void);
void sample_mgr_set_insp_end(void);
//these two functions should be moved to another file
//that handles device context and status
void sample_mgr_set_op_flag(bool flagVal);
bool sample_mgr_get_op_flag(void);
void sample_mgr_set_cycle_sample_counter(uint16_t value);
uint16_t sample_mgr_get_cycle_sample_counter(void);
void sample_mgr_store_hold_start(unsigned long timeRef);
void sample_mgr_store_hold_end(unsigned long timeRef);
float sample_mgr_calc_hold_len(void);
void sample_mgr_calc_hold(void);
float sample_mgr_get_hold_len(void);
float sample_mgr_get_recent_avg(void);
bool sample_mgr_get_insp_seen(void);
void sample_mgr_set_insp_seen(bool state);
void sample_mgr_set_hold_start_flag(bool state);
bool sample_mgr_get_hold_start_flag(void);
void sample_mgr_set_hold_is_zero_flag(bool state);
bool sample_mgr_get_hold_is_zero_flag(void);
bool sample_mgr_get_inside_threshold_flag(void);
void sample_mgr_set_inside_threshold_flag(bool value);
void sample_mgr_set_neg_flow_flag(bool flagVal);
bool sample_mgr_get_neg_flow_flag(void);
void sample_mgr_check_vmax(float v);
float sample_mgr_get_vmax(void);
void sample_mgr_check_vmin(float v);
float sample_mgr_get_vmin(void);
void sample_mgr_reset_vmin_vmax();
void sample_mgr_check_start_measurement();
void sample_mgr_set_exp_start(bool value);
bool sample_mgr_get_exp_start(void);
void sample_mgr_reset_flow_vals(void);
void sample_mgr_accum_neg_flow(float f);
void sample_mgr_accum_pos_flow(float f);
float sample_mgr_get_neg_flow_avg(void);
void sample_mgr_set_new_thresholds(uint8_t thresholdType);
void sample_mgr_reset_peak_flow(void);
void sample_mgr_set_new_sample_flag(bool value);
bool sample_mgr_get_new_sample_flag(void);
bool sample_mgr_get_last_sample(float *p, float *f, float *v);
sample_struct sample_mgr_get_fifo_element_by_id(uint8_t idx);
void sample_mgr_set_tapnea_flag(bool value);
bool sample_mgr_get_tapnea_flag(void);
void sample_mgr_store_sample_in_fifo(float p, float f, float v, uint32_t currentMillis);
uint16_t sample_mgr_get_total_samples_index(void);
void sample_mgr_reset_total_samples_index(void);
void sample_mgr_handle_max_neg_flow(float f);
void sample_mgr_handl_max_pos_flow(float f);
void sample_mgr_reset_max_neg_flow(void);
uint8_t sample_mgr_find_first_sample_to_pos_threshold(void);
uint8_t sample_mgr_find_first_sample_to_neg_threshold(void);
float sample_mgr_get_min_exp_pressure(void);
void sample_mgr_reset_min_neg_pressure(void);
void sample_mgr_initialize_min_exp_pressure(float expPressureInit);
#endif