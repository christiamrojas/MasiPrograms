#include "http.h"
#include "data_mgr.h"
#include "sigfox.h"
#include <ArduinoJson.h>

#define SAMPLE_COUNT_MAX 75
#define SIZE_ARRAYS     22*SAMPLE_COUNT_MAX

char json_to_send[SIZE_ARRAYS] = {'\0'};
alarms_struct active_alarms_data;

sample_measured_vars_struct sample_data;
sample_measured_vars_struct samples_to_send[SAMPLE_COUNT_MAX];
uint8_t sample_count = 0;


void data_mgr_init(void)
{
    memset((void *)&sample_data, 0x00, sizeof(sample_data));
    memset((void *)samples_to_send, 0x00, sizeof(samples_to_send));
    memset((void *)&active_alarms_data, 0x00, sizeof(active_alarms_data));
}

void test_send(void){
    sample_data.alarms[0] = 1;
    sample_data.alarms[1] = 0;
    sample_data.alarms[2] = 1;
    sample_data.operation_mode_set = 2;
    samples_to_send[0].ts_offset = millis();
    sample_count = 2;
    sigfox_send_active_alarms(&sample_data, &active_alarms_data);
    data_mgr_generate_pfv_json_v2();
    publishMessage(json_to_send);
}

void pfv_values_handler(char *data_buffer){
    data_mgr_parse_pfv_sample(data_buffer);
    add_new_sample_to_samples_to_send();

    if(sample_count == SAMPLE_COUNT_MAX){
        data_mgr_generate_pfv_json_v2();
        publishMessage(json_to_send);
        sigfox_send_active_alarms(&sample_data, &active_alarms_data);
        clean_sample_buffer();
    }
}

void measured_vals_handler(char *data_buffer){
    data_mgr_parse_measured_vals_sample(data_buffer);
}

void add_new_sample_to_samples_to_send(void){
    samples_to_send[sample_count++] = sample_data;
}



void data_mgr_parse_pfv_sample(char *dataBuffer)
{
    uint8_t *pPtr, *fPtr, *vPtr;

    pPtr = (uint8_t *)&sample_data.p;
    fPtr = (uint8_t *)&sample_data.f;
    vPtr = (uint8_t *)&sample_data.v;

    for(int i=0; i<4; i++)
    {
        *(pPtr + i) = dataBuffer[4 + i];
        *(fPtr + i)= dataBuffer[8 + i];
        *(vPtr + i) = dataBuffer[12 + i];
    }

    sample_data.ts_offset = (uint32_t)((dataBuffer[16] << 24UL) | (dataBuffer[17] << 16UL) | (dataBuffer[18] << 8UL) | (dataBuffer[19]));

    return;
}

void data_mgr_parse_measured_vals_sample(char *dataBuffer){

    uint8_t *pipPtr, *pmesPtr, *rpmPtr,  *peepPtr, *mvePtr;
    uint8_t *vtiPtr, *vtePtr, *iePtr, *fpiPtr, *fmiPtr, *o2Ptr;
    uint8_t *plateuPtr, *c_staticPtr;

    uint8_t *flow_set_ptr, *weight_set_ptr, *fr_set_ptr, *vt_set_ptr;
    uint8_t *trigger_set_ptr, *fio2_set_ptr, *peep_set_ptr, *pc_set_ptr;
    uint8_t *ti_set_ptr, *ps_set_ptr, *cycling_set_ptr, *tapnea_set_ptr, *operation_mode_ptr;
    uint8_t *c_calculated_ie_ptr;

    pipPtr  = (uint8_t *)&sample_data.PIP;
    pmesPtr = (uint8_t *)&sample_data.Pmes;
    rpmPtr  = (uint8_t *)&sample_data.FR_measured;
    peepPtr = (uint8_t *)&sample_data.PEEP_measured;
    mvePtr  = (uint8_t *)&sample_data.MVe;
    vtiPtr  = (uint8_t *)&sample_data.VTi;
    vtePtr  = (uint8_t *)&sample_data.VTe;
    iePtr   = (uint8_t *)&sample_data.E;
    fpiPtr  = (uint8_t *)&sample_data.FPI;
    fmiPtr  = (uint8_t *)&sample_data.FMI;
    o2Ptr   = (uint8_t *)&sample_data.O2_measured;
    plateuPtr = (uint8_t *)&sample_data.Pplateu;
    c_staticPtr = (uint8_t *)&sample_data.c_static;

    flow_set_ptr = (uint8_t *)&sample_data.flow_set_val;
    weight_set_ptr = (uint8_t *)&sample_data.weight_set_val;
    fr_set_ptr = (uint8_t *)&sample_data.fr_set_val;
    vt_set_ptr = (uint8_t *)&sample_data.vt_set_val;
    trigger_set_ptr = (uint8_t *)&sample_data.trigger_set_val;
    fio2_set_ptr = (uint8_t *)&sample_data.fio2_set_val;
    peep_set_ptr = (uint8_t *)&sample_data.peep_set_val;
    pc_set_ptr = (uint8_t *)&sample_data.pc_set_val;
    ti_set_ptr = (uint8_t *)&sample_data.ti_set_val;
    ps_set_ptr = (uint8_t *)&sample_data.ps_set_val;
    cycling_set_ptr = (uint8_t *)&sample_data.cycling_set_val;
    tapnea_set_ptr = (uint8_t *)&sample_data.tapnea_set_val;
    operation_mode_ptr = (uint8_t *)&sample_data.operation_mode_set;
    c_calculated_ie_ptr = (uint8_t *)&sample_data.c_calculated_ie;

    for(int i=0; i<4; i++)
    {
        *(pipPtr + i)   = dataBuffer[4 + i];
        *(pmesPtr + i)  = dataBuffer[8 + i];
        *(rpmPtr + i)   = dataBuffer[12 + i];
        *(peepPtr + i)  = dataBuffer[16 + i];
        *(mvePtr + i)   = dataBuffer[20 + i];
        *(vtiPtr + i)   = dataBuffer[24 + i];
        *(vtePtr + i)   = dataBuffer[28 + i];
        *(iePtr + i)    = dataBuffer[32 + i];
        *(fpiPtr + i)   = dataBuffer[36 + i];
        *(fmiPtr + i)   = dataBuffer[40 + i];
        *(o2Ptr + i)    = dataBuffer[44 + i];
        *(plateuPtr + i) = dataBuffer[48 + i];
        *(c_staticPtr + i) = dataBuffer[52 + i];
    }

    sample_data.alarms[ALARM_VTMAX_IDX] = dataBuffer[56];
    sample_data.alarms[ALARM_VTMIN_IDX] = dataBuffer[57];
    sample_data.alarms[ALARM_PMAX_IDX]  = dataBuffer[58];
    sample_data.alarms[ALARM_PMIN_IDX]  = dataBuffer[59];
    sample_data.alarms[ALARM_FRMAX_IDX]  = dataBuffer[60];
    sample_data.alarms[ALARM_TAPNEA_IDX]  = dataBuffer[61];
    sample_data.alarms[ALARM_PEEP_MAX_IDX]  = dataBuffer[62];
    sample_data.alarms[ALARM_FIO2_IDX]  = dataBuffer[63];
    sample_data.alarms[ALARM_POSITION_IDX]  = dataBuffer[64];
    sample_data.alarms[ALARM_OVERPRESSURE_IDX]  = dataBuffer[65];
    sample_data.alarms[ALARM_OVERFLOW_IDX]  = dataBuffer[66];
    sample_data.alarms[ALARM_DISCONNECTION_IDX]  = dataBuffer[67];
    sample_data.alarms[ALARM_LEAK_IDX]  = dataBuffer[68];
    sample_data.alarms[ALARM_OCLUSION_IDX]  = dataBuffer[69];
    sample_data.alarms[ALARM_AC_STATUS_IDX]  = dataBuffer[70];
    sample_data.alarms[ALARM_BATTERY_LEVEL_IDX]  = dataBuffer[71];

    for(int i=0; i<4; i++)
    {
        *(flow_set_ptr + i)         = dataBuffer[72 + i];
        *(weight_set_ptr + i)       = dataBuffer[76 + i];
        *(fr_set_ptr + i)           = dataBuffer[80 + i];
        *(vt_set_ptr + i)           = dataBuffer[84 + i];
        *(trigger_set_ptr + i)      = dataBuffer[88 + i];
        *(fio2_set_ptr + i)         = dataBuffer[92 + i];
        *(peep_set_ptr + i)         = dataBuffer[96 + i];
        *(pc_set_ptr + i)           = dataBuffer[100 + i];
        *(ti_set_ptr + i)           = dataBuffer[104 + i];
        *(ps_set_ptr + i)           = dataBuffer[108 + i];
        *(cycling_set_ptr + i)      = dataBuffer[112 + i];
        *(tapnea_set_ptr + i)       = dataBuffer[116 + i];
        *(operation_mode_ptr + i)   = dataBuffer[120 + i];
        *(c_calculated_ie_ptr + i)  = dataBuffer[124 + i];
    }
    return;
}


void data_mgr_generate_pfv_json_v2(void){
    int cursor_str = 0;
    int64_t current_time = (ms_since_epoch - (int64_t)offset_millis) + int64_t(samples_to_send[0].ts_offset);

    cursor_str = sprintf(json_to_send, "{\"dev_id\":\"%02X%02X%02X%02X%02X%02X\",\"samples\":\"%"PRIu64",", 
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], current_time);

    for(int idx=0; idx < sample_count; idx++){
        cursor_str += sprintf(json_to_send + cursor_str, "%.1f,%.1f,%.1f,",
                samples_to_send[idx].p, samples_to_send[idx].f, samples_to_send[idx].v);
    }

    cursor_str += sprintf(json_to_send + cursor_str, "\",\"msrd_vals\":\"%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,",
                    sample_data.PIP, sample_data.Pmes, sample_data.FR_measured, sample_data.PEEP_measured,
                    sample_data.MVe, sample_data.VTi);

    cursor_str += sprintf(json_to_send + cursor_str, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
                    sample_data.VTe, sample_data.E, sample_data.FPI, sample_data.FMI, 
                    sample_data.O2_measured, sample_data.c_static);

    
    cursor_str += sprintf(json_to_send + cursor_str, "\",\"size\":\"%d\"", sample_count);
    cursor_str += sprintf(json_to_send + cursor_str, ",\"alms\":\"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\"",
                        sample_data.alarms[ALARM_BATTERY_LEVEL_IDX], sample_data.alarms[ALARM_AC_STATUS_IDX],
                        sample_data.alarms[ALARM_OCLUSION_IDX], sample_data.alarms[ALARM_LEAK_IDX],
                        sample_data.alarms[ALARM_DISCONNECTION_IDX], sample_data.alarms[ALARM_OVERFLOW_IDX],
                        sample_data.alarms[ALARM_OVERPRESSURE_IDX], sample_data.alarms[ALARM_POSITION_IDX],
                        sample_data.alarms[ALARM_FIO2_IDX], sample_data.alarms[ALARM_PEEP_MAX_IDX],
                        sample_data.alarms[ALARM_TAPNEA_IDX], sample_data.alarms[ALARM_FRMAX_IDX],
                        sample_data.alarms[ALARM_PMIN_IDX], sample_data.alarms[ALARM_PMAX_IDX],
                        sample_data.alarms[ALARM_VTMIN_IDX], sample_data.alarms[ALARM_VTMAX_IDX]);

    cursor_str += sprintf(json_to_send + cursor_str, ",\"set_vals\":\"%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,",
                        sample_data.operation_mode_set, sample_data.flow_set_val, sample_data.weight_set_val,
                        sample_data.fr_set_val, sample_data.vt_set_val, sample_data.trigger_set_val);

    cursor_str += sprintf(json_to_send + cursor_str, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\"}",
                        sample_data.fio2_set_val, sample_data.peep_set_val, sample_data.pc_set_val,
                        sample_data.ti_set_val, sample_data.ps_set_val, sample_data.cycling_set_val,
                        sample_data.tapnea_set_val, sample_data.c_calculated_ie);

    /*
    Serial.println("JSON string input and length: ");
    Serial.println(json_to_send);
    Serial.println(strlen(json_to_send));
    */
    #ifdef TEL_DEBUG
        Debug.println("JSON string input and length: ");
        Debug.println(json_to_send);
        Debug.println(strlen(json_to_send));
    #endif
    
}

void clean_sample_buffer(void){
    memset((void *)samples_to_send, 0x00, sizeof(samples_to_send));
    memset((void *)json_to_send, '\0', sizeof(json_to_send));
    sample_count = 0;
}