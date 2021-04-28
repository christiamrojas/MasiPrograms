#include "commands_manager.h"
#include "context_mgr.h"

unsigned long testWindowStart, testWindowEnd;
unsigned long tailWindowStart, tailWindowEnd;

bool newBatteryData, oldBatteryData;

static autodiag_struct adCmdMgr;

static uint8_t almsToSendArray[ALARM_SIZE];

static bool testForAlarm;

static bool stopFlag;

static float leakThreshold;


void cmd_mgr_init(void)
{
  memset(almsToSendArray, 0, sizeof(almsToSendArray));
  testForAlarm = false;
  stopFlag = true;
  leakThreshold = 100;
}

void cmd_mgr_set_test_for_alarm(bool value)
{
  testForAlarm = value;
}

void cmd_mgr_set_stop_flag(bool value)
{
  stopFlag = value;
}

void cmd_mgr_A1(measured_values_struct *measured_values, ctl_struct result, 
                bool inspIndicator, bool *p_max_flag, bool *p_min_flag){

  float p_max, p_min = 0.0;

  if (result.p > measured_values->PIP)
  {
    measured_values->PIP = result.p;
    p_max = model_def_get_var(ALARM_PMAX).varValue;
    if(measured_values->PIP > p_max){
      *p_max_flag = true;
    }
  }

  measured_values->FMI_acc += result.f;        // Flujo medio inspiratorio
  p_min = model_def_get_var(ALARM_PMIN).varValue;
  
  if((result.p < p_min) && (p_min > 0)){
      *p_min_flag = true;
  }

  sample_mgr_store_sample(result.p, result.f, result.v);
  sample_mgr_write_f_val(result.f, inspIndicator);
  if(!sample_mgr_get_insp_seen())
  {
    sample_mgr_check_ti();
    sample_mgr_check_vmax(result.v);
  }
  else
  {
    sample_mgr_check_vmin(result.v);
  }

  measured_values->t++;

  if ((measured_values->Im==0)&&(measured_values->Em==0)&&(result.f>5)) 
    measured_values->Im = measured_values->Im+1;
  else if ((measured_values->Em==0)&&(result.f<-15)) 
    measured_values->Em = measured_values->Em+1;
  else if ((measured_values->Im>0)&&(measured_values->Em==0)) 
    measured_values->Im= measured_values->Im+1;

}

void cmd_mgr_A3(measured_values_struct *measured_values, ctl_struct result, 
                viewInput *vi, bool *inspIndicator, bool *p_max_flag, 
                bool *p_min_flag){

  float vti_max, vti_min = 0;
  float peep_delta_max = 0.0;
  int alarm_idx = 0;

  switch (result.stage)
  {
      case 1:
      { 
        //Setting time reference for cycle start and cycle end and calculating cycle length
        sample_mgr_set_insp_seen(false);
        unsigned long currentTimeRef = sample_mgr_get_time_ref();
        sample_mgr_set_ti_state(TI_INACTIVE);
        sample_mgr_store_cycle_end(currentTimeRef);
        sample_mgr_calc_cycle();
        sample_mgr_store_cycle_start(currentTimeRef);
        alm_mgr_inc_cycle_counter();
        
        sample_mgr_set_insp_seen(false);

        *inspIndicator = true;

        float cycle = sample_mgr_get_cycle_len();

        if(!context_mgr_get_pause_state())
        {
          vi->ctl_data.start = true;
          vi->ctl_data.active = true;
          vi->ctl_data.pfv_flag = false;
        }
       
        measured_values->t = 0;
        
        if(result.triggeredInsp != 0)
        {
          sample_struct sample = sample_mgr_get_fifo_element_by_id(40); //number of samples before current point. Should be changed to a define
          measured_values->PEEP_measured = sample.p; 
        }
        else
        {
          // measured_values->PEEP_measured = measured_values->p;
          measured_values->PEEP_measured = sample_mgr_get_min_exp_pressure();
        }

        sample_mgr_set_inside_threshold_flag(false);

        adCmdMgr = autodiag_mgr_get();
        if(adCmdMgr.active)
        {
          navigator_struct ns = context_mgr_get_navigator();

          adCmdMgr.cycle_count++;
          autodiag_mgr_set(adCmdMgr);
        }

        measured_values->VTi = sample_mgr_get_vmax();
        float v_min_exp = sample_mgr_get_vmin();
        // VTe = VTi - v;
        if(cycle < 1)
          cycle = 1;

        measured_values->VTe = measured_values->VTi - v_min_exp;

        // TODO: Set the variable leak threshold
        leakThreshold = measured_values->VTi * LEAK_THRESHOLD_PERCENTAGE_VAL;
        if(leakThreshold <= LEAK_ALARM_THRESHOLD_FIXED)
          leakThreshold = LEAK_ALARM_THRESHOLD_FIXED;
        
        measured_values->FR3[2] = measured_values->FR3[1];
        measured_values->FR3[1] = measured_values->FR3[0];

        measured_values->FR3[0] = cycle;

        measured_values->FR_measured = measured_values->FR3[0] + measured_values->FR3[1] + measured_values->FR3[2];
        measured_values->FR_measured = 18000/measured_values->FR_measured;
        measured_values->FR_measured /= 100;

        measured_values->MVe = measured_values->VTi * measured_values->FR_measured/1000;
        measured_values->Em =(measured_values->I + measured_values->E- measured_values->Im);

        float C_static = measured_values->VTi / (measured_values->Pmes - measured_values->PEEP_measured);
        float fio2 = model_def_get_var(FIO2).varValue;
        measured_values->g_suggested_o2 = measured_values->MVe*(fio2/100.0 - 0.21)/0.79;
        measured_values->FPI = sample_mgr_get_max_f();

        //Chequeamos alarmas
        float fr_max = model_def_get_var(ALARM_FRMAX).varValue;
        
        bool tiEndVal = !sample_mgr_get_neg_flow_flag();            
        
        sample_mgr_calc_ti(tiEndVal);
        measured_values->I = sample_mgr_get_ti_len();

        // TODO: Move IE calculation and buffering to its own operation
        #ifdef IE_TIME_PRINT
        Serial.println("-----------------------------------------------------------------");
        Serial.print("Cycle Time: "); Serial.println(cycle);
        Serial.print("Inspiration Time: "); Serial.println(measured_values->I);
        #endif
        
        measured_values->E = cycle - measured_values->I;
        #ifdef IE_TIME_PRINT
        Serial.print("Expiration Time: "); Serial.println(measured_values->E);
        #endif
        measured_values->E = measured_values->E/(measured_values->I); 
        #ifdef IE_TIME_PRINT
        Serial.print("Expiration Time after division: "); Serial.println(measured_values->E);
        #endif
        if(measured_values->E >= 25)
          measured_values->E = 0;

        #ifdef IE_TIME_PRINT
        Serial.println("-----------------------------------------------------------------");
        #endif

        cmd_mgr_ie_fifo_displace(measured_values);
        

        float displayIE;
        if(alm_mgr_get_stability_flag())
          displayIE = cmd_mgr_avg_ie_values(measured_values);
        else
          displayIE = measured_values->E;

        if((measured_values->FR_measured > fr_max) && (testForAlarm)){
          #ifdef SD_FLAG
          sd_log_alarm(LOG_FR_ALARM);
          #endif
          if(!alm_mgr_get_active_status_by_code(ALARM_FRMAX_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_FRMAX_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM  
            comms_mgr_send_alarm(ALARM_FRMAX_ICON_VAL);
            #endif
            almsToSendArray[ALARM_FRMAX_IDX] = 1;
          }// index for FR
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_FRMAX_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_FRMAX_IDX] = 0;
        }
        
        if((measured_values->AC_on == false) && (testForAlarm)){
          if(!alm_mgr_get_active_status_by_code(ALARM_AC_STATUS_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_AC_STATUS_ICON_VAL);
            alm_mgr_activate(alarm_idx); 
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_AC_STATUS_ICON_VAL);
            #endif
            almsToSendArray[ALARM_AC_STATUS_IDX] = 1;
          }
        }
        else{
          if(alm_mgr_get_active_status_by_code(ALARM_AC_STATUS_ICON_VAL) && measured_values->AC_on)
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_AC_STATUS_ICON_VAL);
            alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
            almsToSendArray[ALARM_AC_STATUS_IDX] = 0;
          }
        }

        if((measured_values->position_alarm) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_POSITION_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_POSITION_ICON_VAL);
            alm_mgr_activate(alarm_idx); 
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_POSITION_ICON_VAL);
            #endif
            almsToSendArray[ALARM_POSITION_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_POSITION_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_POSITION_IDX] = 0;
        }

        if((measured_values->over_flow) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_OVERFLOW_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_OVERFLOW_ICON_VAL);
            alm_mgr_activate(alarm_idx); 
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_OVERFLOW_ICON_VAL);
            #endif
            almsToSendArray[ALARM_OVERFLOW_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_OVERFLOW_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_OVERFLOW_IDX] = 0;
        }

        if((measured_values->over_pressure) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_OVERPRESSURE_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_OVERPRESSURE_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_OVERPRESSURE_ICON_VAL);
            #endif
            almsToSendArray[ALARM_OVERPRESSURE_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_OVERPRESSURE_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_OVERPRESSURE_IDX] = 0;
        }

        if((measured_values->voltage_measured < BATTERY_ALARM_THRESHOLD)  && (testForAlarm)){
          if(!alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_BATTERY_LEVEL_ICON_VAL);
            #endif
            almsToSendArray[ALARM_BATTERY_LEVEL_IDX] = 1;
          }
        }
        else{
          if(alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL) && (measured_values->voltage_measured >= BATTERY_ALARM_THRESHOLD))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
            alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
            almsToSendArray[ALARM_BATTERY_LEVEL_IDX] = 0;
          }
        }

        if ((measured_values->disconnection) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG))
        {
          if(!alm_mgr_get_active_status_by_code(ALARM_DISCONNECTION_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_DISCONNECTION_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_DISCONNECTION_ICON_VAL);
            #endif
            almsToSendArray[ALARM_DISCONNECTION_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_DISCONNECTION_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_DISCONNECTION_IDX] = 0;
        }

        // if((measured_values->v > LEAK_ALARM_THRESHOLD_FIXED) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if((abs(measured_values->VTe - measured_values->VTi) > leakThreshold) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_LEAK_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_LEAK_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_LEAK_ICON_VAL);
            #endif
            almsToSendArray[ALARM_LEAK_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_LEAK_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_LEAK_IDX] = 0;
        }

        if((measured_values->obstruction) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG))
        {
          if(!alm_mgr_get_active_status_by_code(ALARM_OCLUSION_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_OCLUSION_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_OCLUSION_ICON_VAL);
            #endif
            almsToSendArray[ALARM_OCLUSION_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_OCLUSION_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_OCLUSION_IDX] = 0;
        }

        float fio2_max = model_def_get_var(ALARM_FIO2_DELTA).varValue;
        float fio2_set = model_def_get_var(FIO2).varValue;

        if((abs(measured_values->O2_measured + 0.5 - fio2_set) > fio2_max) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_FIO2_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_FIO2_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_FIO2_ICON_VAL);
            #endif
            almsToSendArray[ALARM_FIO2_IDX] = 1;
          }
        }
        else{
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_FIO2_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_FIO2_IDX] = 0;
        }

        if((context_mgr_get_current_context() == PC_CSV_CONTEXT) && (measured_values->apneaTrigger) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG))
        {
          if(!alm_mgr_get_active_status_by_code(ALARM_TAPNEA_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_TAPNEA_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef ESP8266_COMM
            comms_mgr_send_alarm(ALARM_TAPNEA_ICON_VAL);
            #endif
            almsToSendArray[ALARM_TAPNEA_IDX] = 1;
          }
        }
        else
        {
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_TAPNEA_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_TAPNEA_IDX] = 0;
        }
        
        if((*p_max_flag) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_PMAX_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_PMAX_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef SD_FLAG
            sd_log_alarm(LOG_PMAX_ALARM);
            #endif
            almsToSendArray[ALARM_PMAX_IDX] = 1;
          }
          *p_max_flag = false;
        }
        else
        {
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_PMAX_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_PMAX_IDX] = 0;
        }

        if((*p_min_flag) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_PMIN_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_PMIN_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef SD_FLAG
            sd_log_alarm(LOG_PMIN_ALARM);
            #endif
            almsToSendArray[ALARM_PMIN_ICON_VAL] = 1;
          }
          *p_min_flag = false;
        }
        else
        {
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_PMIN_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_PMIN_ICON_VAL] = 0;
        }        

        peep_delta_max = model_def_get_var_real_val(model_def_get_var(ALARM_PEEP_DELTA))*10;

        if(((abs(measured_values->PEEP_measured + 0.05 - model_def_get_var(PEEP).varValue)) > peep_delta_max) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG)){
          if(!alm_mgr_get_active_status_by_code(ALARM_PEEP_MAX_ICON_VAL))
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_PEEP_MAX_ICON_VAL);
            alm_mgr_activate(alarm_idx);
            #ifdef SD_FLAG
            sd_log_alarm(LOG_PEEP_ALARM);
            #endif
            almsToSendArray[ALARM_PEEP_MAX_IDX] = 1;
          }
        }
        else
        {
          alarm_idx = alm_mgr_get_idx_by_code(ALARM_PEEP_MAX_ICON_VAL);
          alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          almsToSendArray[ALARM_PEEP_MAX_IDX] = 0;
        }

        sample_mgr_set_tapnea_flag(false);

        // sample_mgr_set_inside_threshold_flag(false);

        if(measured_values->O2_measured < 10 || measured_values->O2_measured > 100)
        {
          measured_values->O2_measured = 0;
        }

        float rpm_theoritical = model_def_get_var(FR).varValue;
        float ti_theoritical = model_def_get_var(TI).varValue; 

        measured_values->g_calculated_ie = (ti_theoritical/((60/rpm_theoritical) - ti_theoritical));

        //This is calculated but has no screen definition yet
        measured_values->resistance = (measured_values->p/(measured_values->v * 1000));

        model_def_store_measuredval_value(measured_values->PIP + 0.5, PIP_VAL);
        model_def_store_measuredval_value(measured_values->Pmes + 0.5, PMES_VAL);
        model_def_store_measuredval_value(measured_values->PEEP_measured + 0.05, PEEP_VAL);
        model_def_store_measuredval_value(measured_values->FR_measured + 0.5, FR_VAL);
        model_def_store_measuredval_value(measured_values->MVe * 10 + 0.5, MVE_VAL);
        model_def_store_measuredval_value(measured_values->VTi + 0.5, VTI_VAL);
        model_def_store_measuredval_value(measured_values->VTe + 0.5, VTE_VAL);
        // model_def_store_measuredval_value(I/10, 7);
        model_def_store_measuredval_value(1, I_VAL);
        // model_def_store_measuredval_value(Em/Im + 0.5, 8);
        
        // model_def_store_measuredval_value(measured_values->E + 0.05, E_VAL);
        model_def_store_measuredval_value(displayIE + 0.05, E_VAL);
        model_def_store_measuredval_value(C_static + 0.5, C_STATIC_VAL);
        model_def_store_measuredval_value(measured_values->FPI + 0.5, PEAK_FLOW_VAL);
        model_def_store_measuredval_value(measured_values->g_suggested_o2 * 10, G_SUGGESTED_O2_FLOW);
        model_def_store_measuredval_value(measured_values->O2_measured, FIO2_VAL);
        // model_def_store_measuredval_value(measured_values->g_calculated_ie, G_CALCULATED_IE);
        model_def_store_measuredval_value(0x3A, COLON_VAL);

        cmd_mgr_set_widget_bulk_struct(&(vi->widgetBulk_data));


        #ifdef ESP8266_COMM_MV
        float measuredValsArray[MEASURED_VALS_NUM_TO_SEND];

        measuredValsArray[0] = measured_values->PIP;
        measuredValsArray[1] = measured_values->Pmes;
        measuredValsArray[2] = measured_values->FR_measured;
        measuredValsArray[3] = measured_values->PEEP_measured;
        measuredValsArray[4] = measured_values->MVe;
        measuredValsArray[5] = measured_values->VTi;
        measuredValsArray[6] = measured_values->VTe;
        measuredValsArray[7] = measured_values->E;
        measuredValsArray[8] = measured_values->FPI;
        measuredValsArray[9] = measured_values->FMI;
        measuredValsArray[10] = measured_values->O2_measured;
        measuredValsArray[11] = measured_values->Pmes;
        measuredValsArray[12] = C_static;

        float setValsArray[SET_VALS_NUM_TO_SEND];
        setValsArray[0] = model_def_get_var(FLOW).varValue;
        setValsArray[1] = model_def_get_var(WEIGHT).varValue;
        setValsArray[2] = model_def_get_var(FR).varValue;
        setValsArray[3] = model_def_get_var(VT).varValue;
        setValsArray[4] = model_def_get_var(TRIGGER).varValue;
        setValsArray[5] = model_def_get_var(FIO2).varValue;
        setValsArray[6] = model_def_get_var(PEEP).varValue;
        setValsArray[7] = model_def_get_var(PC).varValue;
        if(context_mgr_get_current_context() == VC_CMV_CONTEXT)
        {
          setValsArray[8] = model_def_get_var(TIVC).varValue;
          setValsArray[13] = model_def_get_var(C_CALCULATED_IE).varValue;
        }
        else
        {
          setValsArray[8] = model_def_get_var(TI).varValue;
          setValsArray[13] = model_def_get_var(C_CALCULATED_IE_P).varValue;
        }
        setValsArray[9] = model_def_get_var(PS).varValue;
        setValsArray[10] = model_def_get_var(CYCLING).varValue;
        setValsArray[11] = model_def_get_var(TAPNEA).varValue;
        setValsArray[12] = (float)context_mgr_get_current_context();
        
        comms_mgr_send_measured_vals(measuredValsArray, almsToSendArray, setValsArray);
        #endif
       
        measured_values->v = 0;
        measured_values->f = 0;
        measured_values->PIP = measured_values->PEEP_measured;
        measured_values->FPI = 0;
        measured_values->FMI = 0;
        measured_values->FMI_acc = 0;

        sample_mgr_set_cycle_sample_counter(0);
        sample_mgr_set_neg_flow_flag(false);
        
        
        measured_values->I = 0;
        measured_values->E = 0;
        measured_values->HOLD = 0;
        measured_values->Im = measured_values->Em =0;      // Inicio de inspiracion,expiracion medidas
        // peep_delta_max = model_def_get_var(ALARM_PEEP_DELTA).varValue;

        sample_mgr_reset_vmin_vmax();
        uint8_t currentContext = context_mgr_get_current_context();
        if(currentContext == VC_CMV_CONTEXT)
          sample_mgr_set_new_thresholds(AVG);
        else
          sample_mgr_set_new_thresholds(AVG);
        sample_mgr_reset_flow_vals();
        sample_mgr_reset_peak_flow();
        sample_mgr_reset_max_neg_flow();
        sample_mgr_set_exp_start(false);
        sample_mgr_reset_min_neg_pressure();

        break;
      }
      case 2:
        break;
      case 3:
        {
          // sample_mgr_set_insp_end();
          unsigned long currentTimeRef = sample_mgr_get_time_ref();
          *inspIndicator = false;
          if(sample_mgr_get_ti_state())
          {
            sample_mgr_store_alt_ti_end(currentTimeRef);
          }

          if(sample_mgr_get_hold_start_flag())
          {
            sample_mgr_store_hold_end(currentTimeRef);
            sample_mgr_calc_hold();
            measured_values->HOLD = sample_mgr_get_hold_len();
          }
          else
          {
            sample_mgr_set_hold_is_zero_flag(true);
          }
          
          measured_values->t = 0;
          measured_values->Pmes = measured_values->p;
          adCmdMgr = autodiag_mgr_get();
          if(adCmdMgr.active)
          {
            adCmdMgr.pPlateaus[adCmdMgr.pPlateauIdx] = measured_values->Pmes;
            adCmdMgr.pPlateauIdx++;
            adCmdMgr.PIPs[adCmdMgr.PIPIdx] = measured_values->PIP;
            adCmdMgr.PIPIdx++;
            autodiag_mgr_set(adCmdMgr);
          }

          measured_values->FMI = sample_mgr_get_max_f();
      
          vti_max = model_def_get_var(ALARM_VT_MAX).varValue;
          vti_min = model_def_get_var(ALARM_VT_MIN).varValue;

          if((measured_values->VTi > vti_max) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG))
          {
            if(!alm_mgr_get_active_status_by_code(ALARM_VTMAX_ICON_VAL))
            {
              #ifdef SD_FLAG
              sd_log_alarm(LOG_VTMAX_ALARM);
              #endif
              alarm_idx = alm_mgr_get_idx_by_code(ALARM_VTMAX_ICON_VAL);
              alm_mgr_activate(alarm_idx);
              // alm_mgr_activate(2);  //Index for VT MAX
              almsToSendArray[ALARM_VTMAX_IDX] = 1;
            }
          }
          else
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_VTMAX_ICON_VAL);
            alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
            almsToSendArray[ALARM_VTMAX_IDX] = 0;
          }
          
          if((measured_values->VTi < vti_min) && (testForAlarm) && (context_mgr_get_current_context() != AUTODIAG))
          {
            if(!alm_mgr_get_active_status_by_code(ALARM_VTMIN_ICON_VAL))
            {
              #ifdef SD_FLAG
              sd_log_alarm(LOG_VTMIN_ALARM);
              #endif
              alarm_idx = alm_mgr_get_idx_by_code(ALARM_VTMIN_ICON_VAL);
              alm_mgr_activate(alarm_idx);
              // alm_mgr_activate(3);  //Index for VT MIN 
              almsToSendArray[ALARM_VTMIN_IDX] = 1;
            }
          }
          else
          {
            alarm_idx = alm_mgr_get_idx_by_code(ALARM_VTMIN_ICON_VAL);
            alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
            almsToSendArray[ALARM_VTMIN_IDX] = 0;
          }

          sample_mgr_set_exp_start(true);
        }
        break;
      default:
        break;
    }
}

void cmd_mgr_A4(measured_values_struct *measured_values, ctl_struct result){
  //Valores de Oxigeno y Bateria
  measured_values->O2_measured  = result.o2;
  measured_values->voltage_measured = result.volt;
  measured_values->current_measured = result.i;
  measured_values->AC_on = result.ac;
  measured_values->over_pressure = result.over_pressure;
  measured_values->over_flow = result.over_flow;
  measured_values->position_alarm = result.position_alarm;
  measured_values->apneaTrigger = result.apneaTrigger;
  measured_values->sensorDisconnect = result.sensorDisconnect;
  measured_values->disconnection = result.disconnection;
  measured_values->obstruction = result.obstruction;

  newBatteryData = true;
}

widget_struct cmd_mgr_set_widget_struct(measuredVars_struct mvs)
{
	widget_struct ws;
	
	ws.reg = mvs.buttonValue;
  ws.val = mvs.measuredVal;
  ws.decimal = mvs.decimal;
	
	return ws;
}

void cmd_mgr_set_widget_bulk_struct(widgetBulk_struct *wbs)
{
  measuredVars_struct measuredVar;
  measuredVar = model_def_get_measured_var_by_id(PIP_VAL);
  wbs->widget_array[0] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(PMES_VAL);
  wbs->widget_array[1] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(PEEP_VAL);
  wbs->widget_array[2] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(FR_VAL);
  wbs->widget_array[3] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(MVE_VAL);
  wbs->widget_array[4] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(VTI_VAL);
  wbs->widget_array[5] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(VTE_VAL);
  wbs->widget_array[6] = cmd_mgr_set_widget_struct(measuredVar);
  // measuredVar = model_def_get_measured_var_by_id(I_VAL);
  // wbs->widget_array[7] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(E_VAL);
  wbs->widget_array[7] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(C_STATIC_VAL);
  wbs->widget_array[8] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(PEAK_FLOW_VAL);
  wbs->widget_array[9] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(G_SUGGESTED_O2_FLOW);
  wbs->widget_array[10] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(FIO2_VAL);
  wbs->widget_array[11] = cmd_mgr_set_widget_struct(measuredVar);
  // measuredVar = model_def_get_measured_var_by_id(G_CALCULATED_IE);
  // wbs->widget_array[13] = cmd_mgr_set_widget_struct(measuredVar);
  measuredVar = model_def_get_measured_var_by_id(COLON_VAL);
  wbs->widget_array[13] = cmd_mgr_set_widget_struct(measuredVar);
  
  wbs->index = 14;   //Cantidad de elementos cuyos valores deben ser actualizados
  wbs->active = true;
}

bool cmd_mgr_get_new_battery_data(void)
{
  return newBatteryData;
}

void cmd_mgr_set_new_battery_data(bool value)
{
  newBatteryData = value;
}

void cmd_mgr_ie_fifo_displace(measured_values_struct *mvs)
{
  for(int i=IE_FIFO_SIZE-1; i>0; i--)
  {
    mvs->IE_FIFO[i] = mvs->IE_FIFO[i - 1];
  }

  mvs->IE_FIFO[0] = mvs->E;
}

float cmd_mgr_avg_ie_values(measured_values_struct *mvs)
{
  float avg = 0;

  for(int i=0; i<IE_FIFO_SIZE; i++)
  {
    avg += mvs->IE_FIFO[i];
  }

  avg /= IE_FIFO_SIZE;

  return avg;
}

float cmd_mgr_get_fr_avg(measured_values_struct *mvs)
{
  float cycleAvg = 0;

  for(int i=0; i<RPM_FIFO_SIZE; i++)
  {
    cycleAvg += mvs->FR3[i];
  }

  cycleAvg /= RPM_FIFO_SIZE;

  return cycleAvg;
}

bool cmd_mgr_check_cycle_integrity(measured_values_struct *mvs, float currentCycle)
{
  float cycleAvg = 0;
  float acceptedDeviation;

  cycleAvg = cmd_mgr_get_fr_avg(mvs);
  
  acceptedDeviation = cycleAvg * 0.2;

  if(abs(cycleAvg - currentCycle) < acceptedDeviation)
  {
    return true;
  }
  else
  {
    return false;
  }
  
}