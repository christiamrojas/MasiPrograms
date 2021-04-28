#include <stdlib.h>
#include "commands_manager.h"
#include "context_mgr.h"
#include "sd.h"
#include "model.h"


static uint8_t *pPtr, *fPtr, *vPtr;
measured_values_struct measured_values;

static uint8_t screenBufferIndex;

screen_element currentBtn;
screen_element lastBtn;
bool p_max_flag;
bool p_min_flag;

float btnStartingVal;
uint16_t lastValBtnPressed;
uint8_t lastValidVarId;

bool selectFlag;
bool inspIndicator;

uint8_t calcFlag;

bool powerButtonStatus;
bool ventilationStatus;     //This flag shows if ventilation is underway or not

bool backIconChangeFlag;
bool buzzerSentState;
bool silentAlmSent;

unsigned long windowStart, windowEnd;

uint8_t encoderBtnStatus;

static unsigned long batteryCheckWindowStart;

uint8_t system_status_codes;
uint8_t o2_calibration_result_code;

uint8_t last_context;
uint8_t last_stage;

navigator_struct savedNavigator;
bool savedPassInitialState;
uint8_t savedGlobalTarget;
bool savedPauseState;

static uint8_t operationMode;

static autodiag_struct savedAutoDiag;

static uint8_t commsReadBuffer[COMMS_READ_BUFFER_SIZE];
static uint8_t commsReadIndex;

static bool avrCalibrationTableFlag;
static uint8_t avrCalibrationTableBuffer[AVR_CALIBRATION_BUFFER_SIZE];
static uint8_t avrCalibrationTableIndex;

static bool modelPauseState;

void model_init(void)
{
  control_iface_init();
  model_init_screen_elements();
  alm_mgr_init();
  comms_mgr_init();
  cmd_mgr_init();
  p_max_flag = false;
  p_min_flag = false;
  memset(&measured_values, 0, sizeof(measured_values_struct));

  inspIndicator = false;

  screenBufferIndex = 0;
  sample_mgr_init();
  context_mgr_init();
  selectFlag = false; //this shows if a button used with set has been selected

  currentBtn = model_get_btn_data(NO_SEL_VAL); //No button has been selected yet.

  calcFlag = NO_CALC;

  powerButtonStatus = false;
  ventilationStatus = false;

  backIconChangeFlag = false;
  buzzerSentState = false;
  silentAlmSent = false;

  windowStart = 0;
  windowEnd = 0;

  measured_values.apneaTrigger = false;
  system_status_codes = 0xFF;
  o2_calibration_result_code = 0xFF;

  encoderBtnStatus = INIT_STATE;

  model_set_a4_request();

  model_def_set_stop_flag(true);
  batteryCheckWindowStart = millis();

  last_context = 0;
  last_stage = 0;

  model_get_context_state();
  operationMode = -1;

  avrCalibrationTableFlag = false;
  modelPauseState = false;
}

void model_init_screen_elements(void)
{
  var_struct currentVar;

  for(int i=0; i<VAR_NUM; i++)
  {
    currentVar = model_def_get_var_by_idx(i);
    currentVar.varValue = currentVar.defaultVal;
    model_def_set_var(currentVar);
  }
}

int model_handler(modelInput* mo, viewInput* vi)
{
  #ifdef PROFILER_MODEL
    uint32_t start_ctl_iface_handler = millis();
  #endif

  model_ctl_iface_handler(vi);

  #ifdef PROFILER_MODEL
    Serial.print("T.ctl_iface_handler: "); Serial.println(millis()-start_ctl_iface_handler);
    uint32_t start_struct_handler = millis();
  #endif

  model_struct_handler(mo, vi);

  #ifdef PROFILER_MODEL
    Serial.print("T.model_struct_handler: "); Serial.println(millis()-start_struct_handler);
    uint32_t start_alm_mgr_chck_alms = millis();
  #endif


  alm_mgr_chck_alms(&(vi->alarm_data), &(vi->buzzer_data), &(vi->ledCtl_data));
  
  #ifdef PROFILER_MODEL
    Serial.print("T.alm_mgr: "); Serial.println(millis()-start_alm_mgr_chck_alms);
    uint32_t start_screen_buzzer = millis();
  #endif

  #ifndef USE_SCREEN_BUZZER

    if(alm_mgr_get_buzzer_flag() && !buzzerSentState)
    {
      model_send_buzzer_cmd(ALM_BUZZER_ENABLED);
      buzzerSentState = true;
    }
    else if(alm_mgr_get_silent_alm() && !silentAlmSent)
    {
      model_send_buzzer_cmd(ALM_BUZZER_DISABLED);
      silentAlmSent = true;
    }
    else if(!alm_mgr_get_silent_alm() && silentAlmSent)
    {
      model_send_buzzer_cmd(ALM_BUZZER_ENABLED);
      silentAlmSent = false;
    }
    else if(!alm_mgr_get_buzzer_flag() && buzzerSentState)
    {
      model_send_buzzer_cmd(ALM_BUZZER_DISABLED);
      buzzerSentState = false;
    }
  #endif

  #ifdef PROFILER_MODEL
    Serial.print("T.mgr_screen_buzzer: "); Serial.println(millis()-start_screen_buzzer);
    uint32_t start_autodiag_handler = millis();
  #endif

  model_autodiag_handler(vi);

  #ifdef PROFILER_MODEL
    Serial.print("T.model_autodiag_handler: "); Serial.println(millis()-start_autodiag_handler);
    uint32_t start_samples_handler = millis();
  #endif

  model_samples_handler(vi);
  
  #ifdef PROFILER_MODEL
    Serial.print("T.model_samples_handler: "); Serial.println(millis()-start_samples_handler);
    uint32_t start_handle_comms = millis();
  #endif

  #ifdef ESP8266_COMM_PFV
  model_handle_comms(vi);
  #endif

  #ifdef PROFILER_MODEL
    Serial.print("T.model_handle_comms: "); Serial.println(millis()-start_handle_comms);
    uint32_t start_handle_power_cycle = millis();
  #endif

  model_handle_power_cycle(vi);

  #ifdef PROFILER_MODEL
    Serial.print("T.model_handle_power_cycle: "); Serial.println(millis()-start_handle_power_cycle);
    uint32_t start_handle_check_battery = millis();
  #endif

  model_handle_check_battery(vi);

  #ifdef PROFILER_MODEL
    Serial.print("T.model_handle_check_battery: "); Serial.println(millis()-start_handle_check_battery);
    uint32_t start_enc_btn_handler = millis();
  #endif

  model_enc_btn_handler(vi);

  #ifdef PROFILER_MODEL
    Serial.print("T.model_enc_btn_handler: "); Serial.println(millis()-start_enc_btn_handler);
    uint32_t start_handle_stop_operation = millis();
  #endif

  model_handle_stop_operation();

  #ifdef PROFILER_MODEL
    Serial.print("T.model_handle_stop_operation: "); Serial.println(millis()-start_handle_stop_operation);
    uint32_t start_log_handler = millis();
  #endif

  #ifdef SD_FLAG
  model_log_handler();
  #endif

  #ifdef PROFILER_MODEL
    Serial.print("T.model_log_handler: "); Serial.println(millis()-start_log_handler);
    uint32_t start_update_aux_rtc = millis();
  #endif

  #ifndef RTC_FLAG
  #ifndef SCREEN_RTC
  rtc_update_aux_rtc();
  #endif
  #endif

  #ifdef PROFILER_MODEL
    Serial.print("T.rtc_update_aux_rtc: "); Serial.println(millis()-start_update_aux_rtc);
  #endif

  return VIEW;
}

void model_handle_stop_operation(void)
{
  if(model_def_get_stop_flag())
  {
    if((millis() - batteryCheckWindowStart) >= BATTERY_ICON_REFRESH_RATE)
    {
      model_set_a4_request();
      batteryCheckWindowStart = millis();
    }
  }
}

void model_handle_comms(viewInput *vi)
{
  uint16_t sampleIndex;
  sample_struct samplesToSend;

  bool readResult = comms_mgr_read_dataframe(commsReadBuffer, &commsReadIndex);
  

  // Temporal solution for transmitting to AVR device
  if(readResult && commsReadBuffer[3] == 0xBC)
  {
    #ifdef COMMS_INPUT_PRINT
    Serial.println("Got a BC command");
    #endif

    uint32_t epoch = 0;
    for(int i=0; i<4; i++)
    {
      epoch |= (commsReadBuffer[4+i] << (8 * (3 - i)));
    }
    rtc_convert_epoch_to_struct(epoch, vi);
  }
  else if(readResult)
  {
    #ifdef COMMS_INPUT_PRINT
    Serial.println("Incoming data from ESP");
    #endif
    for(int i=0; i<commsReadIndex; i++)
    {
      control_iface_write_byte(commsReadBuffer[i]);
      #ifdef COMMS_INPUT_PRINT
      Serial.print(commsReadBuffer[i], HEX); Serial.print(" ");
      #endif
    }
    #ifdef COMMS_INPUT_PRINT
    Serial.println("");
    #endif
  }

  //This is where we send data to ESP device, this could be placed in a proper function
  if(avrCalibrationTableFlag)
  {
    avrCalibrationTableFlag = false;
    for(int i=0; i<avrCalibrationTableIndex; i++)
      comms_mgr_write_byte(avrCalibrationTableBuffer[i]);
  }

  sampleIndex = sample_mgr_get_total_samples_index();
  if(sampleIndex == PFV_COMMS_COUNTER_THRESHOLD)
  {
    samplesToSend = sample_mgr_get_fifo_element_by_id(0); //grab the latest sample inserted into FIFO
    
    comms_mgr_send_pfv_vals(samplesToSend.p, samplesToSend.f, samplesToSend.v, samplesToSend.millisSinceStart);
    sample_mgr_reset_total_samples_index();
  }
}

void model_autodiag_handler(viewInput *vi)
{
  autodiag_struct ad;

  ad = autodiag_mgr_get(); 

  if(ad.active)
  {
    switch(ad.type)
    {
      case LEAK:
        if((ad.cycle_count == 0) && (!ad.inProgress))
        {
          alm_mgr_set_record_alarm_flag(false);
          model_set_vc_cmv_mode(vi);

          vi->btnCtl_data.lastButtonId = AUTO_DIAG_LEAK_ICON;
          vi->btnCtl_data.lastButtonVal = 0;
          vi->btnCtl_data.currentButtonId = AUTO_DIAG_LEAK_CIRCLE;
          vi->btnCtl_data.currentButtonVal = 1; //Value of check
          vi->btnCtl_data.active = true;
          
          ad.inProgress = true;
          autodiag_mgr_set(ad);
        }
        else if((ad.cycle_count >= TEST_CYCLE_NUM + 1) && (ad.inProgress))
        {
          uint8_t resultIcon = 0;
          float avgPPlateau = 0.0;
          model_set_stop();
          // Deactivates this alarm in case is active since it appears to go off during 
          // operation.
          if(alm_mgr_get_active_status_by_code(ALARM_VTMIN_ICON_VAL))
          {
            uint8_t alarm_idx = alm_mgr_get_idx_by_code(ALARM_VTMIN_ICON_VAL);
            alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
          }
          alm_mgr_set_record_alarm_flag(true);

          for(int i=0; i<TEST_CYCLE_NUM; i++)
          {
            avgPPlateau += ad.pPlateaus[i];
          }
          avgPPlateau /= (float)TEST_CYCLE_NUM;
          
          if(avgPPlateau <= P_PLATEAU_THRESHOLD)
            resultIcon = 61;
          else
            resultIcon = 62;

          vi->btnCtl_data.lastButtonId = AUTO_DIAG_LEAK_CIRCLE;
          vi->btnCtl_data.lastButtonVal = 0;
          vi->btnCtl_data.currentButtonId = AUTO_DIAG_LEAK_ICON;
          vi->btnCtl_data.currentButtonVal = resultIcon; //Value of check
          vi->btnCtl_data.active = true;

          ad.active = false;
          ad.type = NONE;
          ad.inProgress = false;
          ad.cycle_count = 0;
          ad.pPlateauIdx = 0;
          memset(ad.pPlateaus, 0.0, sizeof(ad.pPlateaus));
          memset(ad.PIPs, 0.0, sizeof(ad.PIPs));
          ad.PIPIdx = 0;
          autodiag_mgr_set(ad);
        }
        break;
      case AUD:
        if(ad.audioTestActive)
        {
          if((millis() - ad.buzzerDiagStart) >= BUZZER_BEEP_TIME)
          {
            model_send_buzzer_cmd(0);
            ad.active = false;
            ad.type = NONE;
            ad.audioTestActive = false;
            ad.buzzerDiagStart = 0;
            autodiag_mgr_set(ad);
          }
        }
        else
        {
          ad.audioTestActive = true;
          model_send_buzzer_cmd(1);
          ad.buzzerDiagStart = millis();
          autodiag_mgr_set(ad);
        }
        break;
      case HME:
        if((ad.cycle_count == 0) && (!ad.inProgress))
        {
          alm_mgr_set_record_alarm_flag(false);
          model_set_pc_cmv_mode(vi);
          vi->btnIconCtl_data.btnId = AUTO_DIAG_HME_CIRCLE;
          vi->btnIconCtl_data.btnVal = 1;    //Circle Icon starts moving
          vi->btnIconCtl_data.active = true;

          ad.inProgress = true;
          autodiag_mgr_set(ad);
        }
        else if((ad.cycle_count >= TEST_CYCLE_NUM + 1) && (ad.inProgress))
        {
          float avgPIP = 0.0;
          model_set_stop();
          alm_mgr_set_record_alarm_flag(true);

          for(int i=0; i<TEST_CYCLE_NUM; i++)
          {
            avgPIP += ad.PIPs[i];
          }
          avgPIP /= (float)TEST_CYCLE_NUM;
          
          vi->btnIconCtl_data.btnId = AUTO_DIAG_HME_CIRCLE;
          vi->btnIconCtl_data.btnVal = 0;    //Circle Icon starts moving
          vi->btnIconCtl_data.active = true;          

          vi->widgetBulk_data.widget_array[0].active = true;
          vi->widgetBulk_data.widget_array[0].reg = AUTO_DIAG_HME_NUMERIC;
          vi->widgetBulk_data.widget_array[0].val = avgPIP;
          vi->widgetBulk_data.widget_array[0].decimal = 1;
          vi->widgetBulk_data.index = 1;
          vi->widgetBulk_data.active = true;

          ad.active = false;
          ad.type = NONE;
          ad.inProgress = false;
          ad.cycle_count = 0;
          ad.pPlateauIdx = 0;
          memset(ad.pPlateaus, 0.0, sizeof(ad.pPlateaus));
          memset(ad.PIPs, 0.0, sizeof(ad.PIPs));
          ad.PIPIdx = 0;
          autodiag_mgr_set(ad);
        }
        break;
      case SYSTEM:
        if((!ad.inProgress) && (!ad.sysCheckDone))
        {
          ad.inProgress = true;
          autodiag_mgr_set(ad);

          vi->btnCtl_data.lastButtonId = AUTO_DIAG_SYSTEM_ICON;
          vi->btnCtl_data.lastButtonVal = 0;
          vi->btnCtl_data.currentButtonId = AUTO_DIAG_SYSTEM_CIRCLE;
          vi->btnCtl_data.currentButtonVal = 1;
          vi->btnCtl_data.active = true;
        }
        if((system_status_codes != 0xFF) && (ad.inProgress) && (!ad.sysCheckDone))
        {
          ad.active = false;
          ad.inProgress = false;
          ad.type = NONE;
          ad.sysCheckDone = true;
          autodiag_mgr_set(ad);

          vi->btnCtl_data.lastButtonId = AUTO_DIAG_SYSTEM_CIRCLE;
          vi->btnCtl_data.lastButtonVal = 0;

          // vi->btnIconCtl_data.btnId = AUTO_DIAG_SYSTEM_ICON;
          vi->btnCtl_data.currentButtonId = AUTO_DIAG_SYSTEM_ICON;

          if(system_status_codes == 1)
          {
            // vi->btnIconCtl_data.btnVal = 62;
            vi->btnCtl_data.currentButtonVal = 62;
          }
          else
          {
            // vi->btnIconCtl_data.btnVal = 61;
            vi->btnCtl_data.currentButtonVal = 61;
          }
          // vi->btnIconCtl_data.active = true;
          vi->btnCtl_data.active = true;
        }
        break;
      case O2_HIGH:
        if((ad.cycle_count == 0) && (!ad.inProgress))
        {    
          alm_mgr_set_record_alarm_flag(false);
          model_set_o2_calibration_operation();
          
          vi->btnCtl_data.lastButtonId = AUTO_DIAG_O2_HIGH_ICON;
          vi->btnCtl_data.lastButtonVal = 0;
          vi->btnCtl_data.currentButtonId = AUTO_DIAG_O2_HIGH_CIRCLE;
          vi->btnCtl_data.currentButtonVal = 1;
          vi->btnCtl_data.active = true;
          
          ad.inProgress = true;
          ad.o2_test_stage = 0;
          autodiag_mgr_set(ad);
          savedAutoDiag = ad;
        }
        else if((ad.cycle_count >= O2_CALIBRATION_CYCLE_NUM) && (ad.o2_test_stage == 0))
        {
          alm_mgr_set_record_alarm_flag(true);

          navigator_struct ns = context_mgr_get_navigator();
        
          vi->btnCtl_data.active = true;

          currentBtn = model_get_btn_data(0);

          ad = savedAutoDiag;

          ad.active = false;
          // ad.inProgress = false;
          ad.type = NONE;
          ad.cycle_count = 0;
          ad.pPlateauIdx = 0;
          memset(ad.pPlateaus, 0.0, sizeof(ad.pPlateaus));
          memset(ad.PIPs, 0.0, sizeof(ad.PIPs));
          ad.PIPIdx = 0;
          ad.o2_test_stage = 1;
          ad.o2_test = true;
          autodiag_mgr_set(ad);
        }
        break;
      case O2_LOW:
        if((ad.cycle_count == 0) && (!ad.inProgress))
        {
          alm_mgr_set_record_alarm_flag(false);
          model_set_o2_calibration_operation();

          ad.inProgress = true;
          autodiag_mgr_set(ad);

          savedAutoDiag = ad;

          vi->btnCtl_data.lastButtonId = AUTO_DIAG_O2_LOW_ICON;
          vi->btnCtl_data.lastButtonVal = 0;
          vi->btnCtl_data.currentButtonId = AUTO_DIAG_O2_LOW_CIRCLE;
          vi->btnCtl_data.currentButtonVal = 1;
          vi->btnCtl_data.active = true;

        }
        else if((ad.cycle_count >= O2_CALIBRATION_CYCLE_NUM))
        {
          alm_mgr_set_record_alarm_flag(true);

          vi->btnCtl_data.active = true;

          ad = savedAutoDiag;

          ad.active = false;
          ad.type = NONE;
          ad.cycle_count = 0;
          ad.pPlateauIdx = 0;
          // ad.inProgress = false;
          memset(ad.pPlateaus, 0.0, sizeof(ad.pPlateaus));
          memset(ad.PIPs, 0.0, sizeof(ad.PIPs));
          ad.PIPIdx = 0;
          ad.o2_test_stage = 1;
          ad.o2_test = true;
          autodiag_mgr_set(ad);
        }
        break;
      default:
        break;
    }
  }
}

void model_enc_btn_handler(viewInput *vi)
{
  autodiag_struct ad;
  ad = autodiag_mgr_get();
  if(encoderBtnStatus == BTN_RELEASED)
  {
    uint8_t currentContext = context_mgr_get_current_context();
    uint8_t targetContext = context_mgr_get_target_context();
    if(model_check_conf_values(targetContext) && !ad.o2_test)
    {
      //TODO: change var update to a function, this code block is 
      //repeated on multiple parts
      
      var_struct var = model_def_get_var(currentBtn.varId);
      model_def_set_var(var);
      model_update_widget_array(var, &(vi->widgetBulk_data), SET);
      model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
    
      vi->btnCtl_data.lastButtonId = currentBtn.buttonSelect;
      vi->btnCtl_data.lastButtonVal = currentBtn.offValue;

      currentBtn = model_get_btn_data(NO_SEL_VAL);

      vi->btnCtl_data.currentButtonId = currentBtn.buttonSelect;
      vi->btnCtl_data.currentButtonVal = currentBtn.offValue;

      vi->btnCtl_data.active = true;

      selectFlag = false;

      switch(currentContext)
      {
        case VC_CMV_CONTEXT:
          model_set_vc_cmv_mode(vi);
          break;
        case PC_CMV_CONTEXT:
          model_set_pc_cmv_mode(vi);
          break;
        case PC_CSV_CONTEXT:
          model_set_pc_csv_mode(vi);
          break;
        case VC_CMV_CONF:
        case PC_CMV_CONF:
        case PC_CSV_CONF:
          break;
        default:
          break;
      }
    }
    else if(ad.o2_test)
    {
      model_set_stop();
      vi->btnCtl_data.lastButtonVal = 0;
      if(ad.o2_high_active)       
      {
        vi->btnCtl_data.lastButtonId = AUTO_DIAG_O2_HIGH_CIRCLE;
        vi->btnCtl_data.currentButtonId = AUTO_DIAG_O2_HIGH_ICON;
      }
      else
      {
        vi->btnCtl_data.lastButtonId = AUTO_DIAG_O2_LOW_CIRCLE;
        vi->btnCtl_data.currentButtonId = AUTO_DIAG_O2_LOW_ICON;
      }
      vi->btnCtl_data.currentButtonVal = 62; //Value of check
      vi->btnCtl_data.active = true;
      
      uint8_t data[1];
      data[0] = ad.o2_config_val;
      control_iface_pack(0xB8, data);

      ad.active = false;
      ad.type = NONE;
      ad.o2_test = false;
      ad.o2_high_active = false;
      ad.o2_low_active = false;
      ad.o2_test_stage = 0;
      ad.inProgress = false;
      autodiag_mgr_set(ad);
    }
    else
    {
      var_struct var = model_def_get_var(lastValidVarId);
      var.varValue = btnStartingVal;
      model_def_set_var(var);
      model_update_widget_array(var, &(vi->widgetBulk_data), SET);
      model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
      
      model_calc_theoretical_ie(&(vi->widgetBulk_data));
      vi->widgetBulk_data.active = true;
    }
    encoderBtnStatus = INIT_STATE;
  }
}

void model_send_buzzer_cmd(uint8_t buzzerState)
{
  uint8_t data[1];

  data[0] = buzzerState;

  control_iface_pack(0xB6, data);
}

void model_handle_check_battery(viewInput *vi)
{
  uint16_t iconVal;
  int alarm_idx;

  uint8_t currentContext = context_mgr_get_current_context();

  if(cmd_mgr_get_new_battery_data())
  {
    if(measured_values.AC_on)
    {
      iconVal = BATTERY_CHARGING;
      // This will turn off both possible alarms for AC Status and battery if they are active 
      // and if an AC source has been plugged
      if(alm_mgr_get_active_status_by_code(ALARM_AC_STATUS_ICON_VAL))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_AC_STATUS_ICON_VAL);
        alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
      }
    }
    else if(measured_values.voltage_measured >= 26.0) //Bateria totalmente cargada
    {
      iconVal = BATTERY_FULL;
      if(!alm_mgr_get_active_status_by_code(ALARM_AC_STATUS_ICON_VAL) && (currentContext != AUTODIAGNOSTIC))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_AC_STATUS_ICON_VAL);
        alm_mgr_activate(alarm_idx); 
        #ifdef ESP8266_COMM
        comms_mgr_send_alarm(ALARM_AC_STATUS_ICON_VAL);
        #endif
      }
      if(alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
        alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
      }
    }
    else if(measured_values.voltage_measured >= 25.0)
    {
      iconVal = BATTERY_HIGH;
      if(!alm_mgr_get_active_status_by_code(ALARM_AC_STATUS_ICON_VAL) && (currentContext != AUTODIAGNOSTIC))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_AC_STATUS_ICON_VAL);
        alm_mgr_activate(alarm_idx); 
        #ifdef ESP8266_COMM
        comms_mgr_send_alarm(ALARM_AC_STATUS_ICON_VAL);
        #endif
      }
      if(alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
        alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
      }
    }
    else if(measured_values.voltage_measured >= 24.0)
    {
      iconVal = BATTERY_MED;
      if(!alm_mgr_get_active_status_by_code(ALARM_AC_STATUS_ICON_VAL) && (currentContext != AUTODIAGNOSTIC))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_AC_STATUS_ICON_VAL);
        alm_mgr_activate(alarm_idx); 
        #ifdef ESP8266_COMM
        comms_mgr_send_alarm(ALARM_AC_STATUS_ICON_VAL);
        #endif
      }
      if(alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
        alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), alarm_idx);
      }
    }
    else if(measured_values.voltage_measured >= 23.0)
    {
      iconVal = BATTERY_LOW;
      if(!alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL) && (currentContext != AUTODIAGNOSTIC))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
        alm_mgr_activate(alarm_idx);
        #ifdef ESP8266_COMM
        comms_mgr_send_alarm(ALARM_BATTERY_LEVEL_ICON_VAL);
        #endif
      }
    }
    else
    {
      iconVal = NO_BATTERY;
      if(!alm_mgr_get_active_status_by_code(ALARM_BATTERY_LEVEL_ICON_VAL) && (currentContext != AUTODIAGNOSTIC))
      {
        alarm_idx = alm_mgr_get_idx_by_code(ALARM_BATTERY_LEVEL_ICON_VAL);
        alm_mgr_activate(alarm_idx);
        #ifdef ESP8266_COMM
        comms_mgr_send_alarm(ALARM_BATTERY_LEVEL_ICON_VAL);
        #endif
      }
    }

    
    vi->batteryIcon_data.iconVal = iconVal;
    vi->batteryIcon_data.active = true;

    cmd_mgr_set_new_battery_data(false);
  }
}

void model_handle_power_cycle(viewInput* vi)
{
  #ifdef SHUTDOWN_SEQ_NEW
    autodiag_struct ads = autodiag_mgr_get();

    // this should prevent shutdown while autodiagnostic operation is in effect
    if(ads.inProgress)
    {
      powerButtonStatus = false;
      return;
    }

    if(powerButtonStatus)
    {
      if(!model_check_context_integrity())
      model_set_context_state();

      context_mgr_set_shutdown_error_context();
      vi->screen_data.active = true;
      vi->screen_data.screenId = context_mgr_get_current_screen_id();

      model_get_context_state();
      powerButtonStatus = false;
    }
  #else
    if(powerButtonStatus && !ventilationStatus)
    {
      uint16_t data[1];
      
      data[0] = 0x55;
      control_iface_pack(0xB5, (uint8_t *)data);
      powerButtonStatus = false;
    }
    else if(ventilationStatus)
    {
      powerButtonStatus = false;
    }
  #endif
}

void model_set_off_values(viewInput* vi)
{
  vi->multiBtnCtl_data.index = ELEMENT_NUM;
  for(int i=0; i<ELEMENT_NUM; i++)
  {
    screen_element scrElement;
    scrElement = model_def_get_element_by_idx(i);
    if(scrElement.buttonValue != BACK_VAL)
    {
      vi->multiBtnCtl_data.btnArray[i].btnId = scrElement.buttonSelect;
      vi->multiBtnCtl_data.btnArray[i].btnVal = scrElement.offValue;
    }
  }
}

void model_get_all_elements(viewInput* vi)
{
  uint8_t num;
  var_struct var;
  screen_element scrElement;

  num = 0;

  for(int i=0; i<ELEMENT_NUM; i++)
  {
    scrElement = model_def_get_element_by_idx(i);
    var = model_def_get_var(scrElement.varId);

    vi->widgetBulk_data.widget_array[i].reg = scrElement.buttonValue;
    vi->widgetBulk_data.widget_array[i].val = var.varValue;
    vi->widgetBulk_data.widget_array[i].decimal = var.decimal;

    num = i;
  }

  vi->widgetBulk_data.index = num;
  vi->widgetBulk_data.active = true;
}

void model_restore_reset_values(viewInput* vi, uint16_t *values)
{
  float valueToReset;
  
  vi->screen_data.screenId = values[0];
  vi->screen_data.active = true;

  valueToReset = (float)(values[1]);
  model_set_widget_to_reset_value(TIVC, valueToReset, vi);

  valueToReset = (float)(values[2]);
  model_set_widget_to_reset_value(VT, valueToReset, vi);

  valueToReset = (float)(values[3]);
  model_set_widget_to_reset_value(FR, valueToReset, vi);

  valueToReset = (float)(values[4]);
  model_set_widget_to_reset_value(TRIGGER, valueToReset, vi);

  valueToReset = (float)(values[5]);
  model_set_widget_to_reset_value(PEEP, valueToReset, vi);

  valueToReset = (float)(values[6]);
  model_set_widget_to_reset_value(FIO2, valueToReset, vi);

  valueToReset = (float)(values[7]);
  model_set_widget_to_reset_value(PC, valueToReset, vi);

  valueToReset = (float)(values[8]);
  model_set_widget_to_reset_value(TI, valueToReset, vi);

  valueToReset = (float)(values[9]);
  model_set_widget_to_reset_value(PS, valueToReset, vi);

  valueToReset = (float)(values[10]);
  model_set_widget_to_reset_value(CYCLING, valueToReset, vi);

  valueToReset = (float)(values[11]);
  model_set_widget_to_reset_value(TAPNEA, valueToReset, vi);

  valueToReset = (float)(values[12]);
  model_set_widget_to_reset_value(ALARM_PMAX, valueToReset, vi);

  valueToReset = (float)(values[13]);
  model_set_widget_to_reset_value(ALARM_PMIN, valueToReset, vi);

  valueToReset = (float)(values[14]);
  model_set_widget_to_reset_value(ALARM_VT_MAX, valueToReset, vi);

  valueToReset = (float)(values[15]);
  model_set_widget_to_reset_value(ALARM_VT_MIN, valueToReset, vi);

  valueToReset = (float)(values[16]);
  model_set_widget_to_reset_value(ALARM_FRMAX, valueToReset, vi);

  valueToReset = (float)(values[17]/10.0);
  model_set_widget_to_reset_value(ALARM_PEEP_DELTA, valueToReset, vi);

  valueToReset = (float)(values[19]);
  model_set_widget_to_reset_value(ALARM_FIO2_DELTA, valueToReset, vi);

  vi->widgetBulk_data.active = true;
}

float model_get_v_theoretical_ie(void)
{
  float inspTime = model_def_get_var(TIVC).varValue;
  float rpm = model_def_get_var(FR).varValue;

  inspTime = inspTime / 10.0;

  float ie = inspTime/((60/rpm) - inspTime);
  ie = (1.0 / ie) + 0.5;

  return ie;
}

float model_get_p_theoretical_ie(void)
{
  float inspTime = model_def_get_var(TI).varValue;
  float rpm = model_def_get_var(FR).varValue;

  inspTime = inspTime / 10.0;

  float ie = inspTime/((60/rpm) - inspTime);
  ie = (1.0 / ie) + 0.5;

  return ie;
}

void model_set_widget_to_reset_value(uint16_t varId, float value, viewInput *vi)
{
  var_struct var = model_def_get_var(varId);
  var.varValue = value;
  model_update_widget_array(var, &(vi->widgetBulk_data), SET);
  model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
  model_def_set_var(var);

}

void model_log_handler()
{
  int logIndex = sd_get_pfv_array_index();

  if(logIndex >= SD_PFV_BUF_SIZE)
  {
    sd_log_pfv();
  }
}

void model_struct_handler(modelInput *mo, viewInput *vi)
{
  if(mo->button_data.active){
    model_handle_btn_cmd(mo, vi);
    model_handle_context(vi);
    model_handle_device_operation(vi);
    mo->button_data.active = false;
  }
  if(mo->encInput_data.active){
    model_handle_encoder_cmd(mo, vi);
    mo->encInput_data.active = false;
  }
}

void model_samples_handler(viewInput *vi)
{
  // if(!context_mgr_get_pause_state())
  if(!modelPauseState)
  {
    #ifdef USE_ALL_SAMPLES
    if(sample_mgr_get_new_sample_flag())
    {
      // windowEnd = millis();
      float pValue, fValue, vValue;
      if(sample_mgr_get_last_sample(&pValue, &fValue, &vValue)){
          uint8_t currentContext = context_mgr_get_current_context();
          vi->ctl_data.p = pValue;
          vi->ctl_data.f = fValue;
          vi->ctl_data.v = vValue;
          vi->ctl_data.active = true;
          vi->ctl_data.start = false;
          if((currentContext == VC_CMV_CONTEXT) || (currentContext == PC_CMV_CONTEXT) || (currentContext == PC_CSV_CONTEXT))
            vi->ctl_data.pfv_flag = true;
      }
      sample_mgr_set_new_sample_flag(false);

      // windowStart = millis();
    }
    #else
    if(sample_mgr_get_write_flag())
    {
      float pValue, fValue, vValue;
      int index = sample_mgr_get_write_index();
      sample_mgr_get_samples_by_idx(index, &pValue, &fValue, &vValue);
      vi->ctl_data.p = pValue;
      vi->ctl_data.f = fValue;
      vi->ctl_data.v = vValue;
      vi->ctl_data.active = true;
      vi->ctl_data.start = false;
      vi->ctl_data.pfv_flag = true;
    }
    sample_mgr_set_new_sample_flag(false);
    #endif
  }
}

void model_ctl_iface_handler(viewInput *vi)
{
  ctl_struct result;

  result = model_check_ctl_iface();
  if(result.active)
  { 
    switch (result.cmd)
    {
      case 0xA1:
        cmd_mgr_A1(&measured_values, result, inspIndicator, &p_max_flag, 
                    &p_min_flag);
        break;
      case 0xA3:
        cmd_mgr_A3(&measured_values, result, vi, &inspIndicator, &p_max_flag, 
                    &p_min_flag);
        break;
      case 0xA4:
        cmd_mgr_A4(&measured_values, result);
        break;
      
      default:
        break;
    }
  }
}

void  model_handle_btn_cmd(modelInput* mo, viewInput* vi)
{
  lastBtn = currentBtn;

  vi->btnCtl_data.lastButtonId = lastBtn.buttonSelect;
  vi->btnCtl_data.lastButtonVal = lastBtn.offValue;

  Serial.print("Button pressed: "); Serial.println(mo->button_data.buttonId, HEX);

  // if(mo->button_data.buttonId >=0x300 && mo->button_data.buttonId <= 0x400)
  if(model_check_if_btn_reset(mo->button_data.buttonId) || model_check_if_btn_reset(lastBtn.buttonValue))
  {
    if((mo->button_data.buttonId == VC_CMV_SET_VAL) || (mo->button_data.buttonId == PC_CMV_SET_VAL) || (mo->button_data.buttonId == PC_CSV_SET_VAL))
      selectFlag = false;

    if(selectFlag)
    {
      var_struct var = model_def_get_var(lastValidVarId);
      var.varValue = btnStartingVal;
      model_def_set_var(var);
      model_update_widget_array(var, &(vi->widgetBulk_data), SET);
      model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);

      if((calcFlag == IE_CALC) || (calcFlag == ALL_CALC))
      { 
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
      }

      if((calcFlag == O2_CALC) || (calcFlag == ALL_CALC))
        model_calc_theoretical_o2(&(vi->widgetBulk_data));
    }
    selectFlag = true;
  }
 
  currentBtn = model_get_btn_data(mo->button_data.buttonId);
  if(currentBtn.varId != 0)
  {
    lastValidVarId = currentBtn.varId;
    btnStartingVal = model_def_get_var(currentBtn.varId).varValue;
    lastValBtnPressed = currentBtn.buttonValue;
  }

  vi->btnCtl_data.currentButtonId = currentBtn.buttonSelect;
  vi->btnCtl_data.currentButtonVal = currentBtn.onValue;

  vi->btnCtl_data.active = true;
}

bool model_check_if_btn_reset(uint16_t btnId)
{
  bool checkResult;
  // This basically acts as a LUT to check if the button ID should be reset or not
  switch(btnId)
  {
    case WEIGHT_VAL:
    case FLOW_CONF_VAL:
    case FLOW_SET_VAL:
    case FR_CONF_VAL:
    case FR_SET_VAL:
    case VT_CONF_VAL:
    case VT_SET_VAL:
    case TRIGGER_CONF_VAL:
    case TRIGGER_SET_VAL:
    case FIO2_CONF_VAL:
    case FIO2_SET_VAL:
    case PEEP_CONF_VAL:
    case PEEP_SET_VAL:
    case PC_CONF_VAL:
    case PC_SET_VAL:
    case TIVC_CONF_VAL:
    case TIVC_SET_VAL:
    case TI_SET_VAL:
    case TI_CONF_VAL:
    case PS_CONF_VAL:
    case PS_SET_VAL:
    case CICLYNG_CONF_VAL:
    case CICLYNG_SET_VAL:
    case TAPNEA_CONF_VAL:
    case TAPNEA_SET_VAL:
    case ALMPMAX_VAL:
    case ALMPMIN_VAL:
    case ALMVTMAX_VAL:
    case ALMVTMIN_VAL:
    case ALMFRMAX_VAL:
    case ALMPEEP_VAL:
    case ALMFIO2_VAL:
      checkResult = true;
      break;
    default:
      checkResult = false;
      break;
  }

  return checkResult;  
}


//TODO: This function needs refactoring to account for use of same function in different parts
void model_handle_device_operation(viewInput *vi)
{
  switch(currentBtn.buttonValue)
  {
    case VC_CMV_SET_VAL:
    {
      if(model_check_conf_values(VC_CMV_CONTEXT))
      {
        model_set_vc_cmv_mode(vi);
        sample_mgr_check_start_measurement();
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        calcFlag = NO_CALC;
      }
      else
      {
        var_struct var = model_def_get_var(lastValidVarId);
        var.varValue = btnStartingVal;
        model_def_set_var(var);
        model_update_widget_array(var, &(vi->widgetBulk_data), SET);
        model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
        
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        vi->widgetBulk_data.active = true;
      }
      
      break;
    }
    case PC_CMV_SET_VAL:
    { 
      if(model_check_conf_values(PC_CMV_CONTEXT))
      {
        model_set_pc_cmv_mode(vi);
        sample_mgr_check_start_measurement();
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        calcFlag = NO_CALC;
      }
      else
      {
        var_struct var = model_def_get_var(lastValidVarId);
        var.varValue = btnStartingVal;
        model_def_set_var(var);
        model_update_widget_array(var, &(vi->widgetBulk_data), SET);
        model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
        
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        vi->widgetBulk_data.active = true;
      }
      
      break;
    }
    case PC_CSV_SET_VAL:
    {
      if(model_check_conf_values(PC_CSV_CONTEXT))
      {
        model_set_pc_csv_mode(vi);
        sample_mgr_check_start_measurement();
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        calcFlag = NO_CALC;
      }
      else
      {
        var_struct var = model_def_get_var(lastValidVarId);
        var.varValue = btnStartingVal;
        model_def_set_var(var);
        model_update_widget_array(var, &(vi->widgetBulk_data), SET);
        model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
        
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        vi->widgetBulk_data.active = true;
      }
      
      break;
    }
    case STOP_VAL:
    case VC_CMV_OK_VAL:
    case PC_CMV_OK_VAL:
    case PC_CSV_OK_VAL:
      model_set_stop();
      sample_mgr_set_op_flag(DEV_STOPPED);
      calcFlag = NO_CALC;
      break;
    case ALARM_VAL:
      alm_mgr_deactivate(&(vi->alarm_data), &(vi->buzzer_data), CURRENT_ALARM_FLAG);
      calcFlag = NO_CALC;
      break;
    case SILENT_ALM_VAL:
      if(alm_mgr_get_silent_alm())
      {
        alm_mgr_set_silent_alm(false);
      }
      else
      {
        if(alm_mgr_get_num_alarms_active() > 0)
        {
          alm_mgr_set_silent_alm(true);
          alm_mgr_start_silent_timer();
          silentAlmSent = false;
        }
      }
      calcFlag = NO_CALC;
      break;
    case TI_CONF_VAL:
    case TIVC_CONF_VAL:
    case TI_SET_VAL:
    case TIVC_SET_VAL:
      calcFlag = IE_CALC;
      break;
    case VT_CONF_VAL:
    case FIO2_CONF_VAL:
    case VT_SET_VAL:
    case FIO2_SET_VAL:
      calcFlag = O2_CALC;
      break;
    case FR_CONF_VAL:
    case FR_SET_VAL: 
      calcFlag = ALL_CALC;
      break;
    case PAUSE_VAL:
      model_toggle_pause_state(vi);
      break;
    case AUTO_DIAG_LEAK_VAL:
      cmd_mgr_set_test_for_alarm(false);
      autodiag_struct ad;
      ad = autodiag_mgr_get();
      ad.active = true;
      ad.type = LEAK;
      ad.inProgress = false;
      ad.cycle_count = 0;
      autodiag_mgr_set(ad);
      break;
    case AUTO_DIAG_AUD_VAL:
      cmd_mgr_set_test_for_alarm(false);
      ad = autodiag_mgr_get();
      ad.active = true;
      ad.type = AUD;
      autodiag_mgr_set(ad);
      break;
    case AUTO_DIAG_HME_VAL:
      cmd_mgr_set_test_for_alarm(false);
      ad = autodiag_mgr_get();
      ad.active = true;
      ad.type = HME;
      ad.inProgress = false;
      ad.cycle_count = 0;
      autodiag_mgr_set(ad);
      break;
    case AUTODIAG_AUD_OK:
      vi->btnIconCtl_data.btnId = AUTO_DIAG_AUD_ICON;
      vi->btnIconCtl_data.btnVal = 62;
      vi->btnIconCtl_data.active = true;
      break;
    case AUTODIAG_AUD_NO_OK:
      vi->btnIconCtl_data.btnId = AUTO_DIAG_AUD_ICON;
      vi->btnIconCtl_data.btnVal = 61;
      vi->btnIconCtl_data.active = true;
      break;
    case AUTO_DIAG_SYSTEM_VAL:
      {
        cmd_mgr_set_test_for_alarm(false);
        ad = autodiag_mgr_get();
        ad.active = true;
        ad.type = SYSTEM;
        autodiag_mgr_set(ad);
        uint8_t data[1];
        data[0] = 0x00;
        control_iface_pack(0xB9, data);
      }
      break;
    case AUTO_DIAG_O2_HIGH_VAL:
      {
        cmd_mgr_set_test_for_alarm(false);
        ad = autodiag_mgr_get();
        ad.active = true;
        ad.type = O2_HIGH;
        ad.o2_test = true;
        ad.o2_high_active = true;
        ad.o2_low_active = false;
        ad.inProgress = false;
        ad.cycle_count = 0;
        ad.o2_config_val = AUTODIAG_O2_MIN;
        autodiag_mgr_set(ad);
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].active = true;
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].reg = AUTO_DIAG_O2_HIGH_NUMERIC;
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].val = ad.o2_config_val;
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].decimal = 0;
        vi->widgetBulk_data.index++;
        vi->widgetBulk_data.active = true;
        break;
      }
    case AUTO_DIAG_O2_LOW_VAL:
      {
        cmd_mgr_set_test_for_alarm(false);
        ad = autodiag_mgr_get();
        ad.active = true;
        ad.type = O2_LOW;
        ad.o2_test = true;
        ad.o2_high_active = false;
        ad.o2_low_active = true;
        ad.inProgress = false;
        ad.cycle_count = 0;
        ad.o2_config_val = AUTODIAG_O2_LOW_VAL;
        autodiag_mgr_set(ad);
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].active = true;
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].reg = AUTO_DIAG_O2_LOW_NUMERIC;
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].val = ad.o2_config_val;
        vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].decimal = 0;
        vi->widgetBulk_data.index++;
        vi->widgetBulk_data.active = true;
        break;
      }
    case BACK_VAL:
      if(context_mgr_get_trigger_ie_calc())
      {
        vi->screen_data.active = true;
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
        vi->widgetBulk_data.active = true;
        context_mgr_set_trigger_ie_calc(false);
        ad = autodiag_mgr_get();
        ad.active = false;
        ad.o2_test = false;
        ad.o2_high_active = false;
        ad.o2_low_active = false;
        autodiag_mgr_set(ad);
      }
      break;
    case SHUTDOWN_VAL:
      uint8_t data[1];
      data[0] = 0x55;
      control_iface_pack(0xB5, (uint8_t *)data);
      powerButtonStatus = false;
      break;
    default:
      calcFlag = NO_CALC;
      break;
  }
}

void model_deselect_current_btn(viewInput *vi)
{
  vi->btnIconCtl_data.active = true;
  vi->btnIconCtl_data.btnId = lastBtn.buttonSelect;
  vi->btnIconCtl_data.btnVal = lastBtn.offValue;
}

//TODO: This function needs refactoring to account for use of same function in different parts
void model_handle_context(viewInput *vi)
{
  if(!model_check_context_integrity())
  {
    model_set_context_state();
  }

  uint8_t currentStage = context_mgr_get_stage();
  #ifdef BUTTON_STAGE_PRINT
  Serial.println("=================================");
  Serial.print("Button Value: "); Serial.println(currentBtn.buttonValue, HEX);
  Serial.print("Current Stage: "); Serial.println(currentStage);
  Serial.println("=================================");
  #endif
  if(((currentBtn.buttonValue == NEXT_VAL) && (currentStage== CONF)) || ((currentBtn.buttonValue == NEXT_VAL) && (currentStage== ALM)))
  {
    uint8_t opmode = context_mgr_get_target_context();
    bool checkConf = model_check_conf_values(opmode);
    context_mgr_set_conf_check(checkConf);
  }
  bool contextResult = context_mgr_check_context(currentBtn.buttonValue);
  uint8_t currentContext = context_mgr_get_current_context();

  if(contextResult)
  {
    switch(currentBtn.buttonValue)
    {
        case VC_CMV_VAL:
          if((currentContext == VC_CMV_CONF) || (currentContext == PC_CMV_CONF) || (currentContext == PC_CSV_CONF))
          {
            model_def_restore_vars_from_backup();
            model_deselect_current_btn(vi);
            model_restore_conf_btns(vi);
          }
          model_def_backup_vars();
          context_mgr_start_context_selection(VC_CMV_CONTEXT, VC_CMV_CONF);
          break;
        case PC_CMV_VAL:
          if((currentContext == VC_CMV_CONF) || (currentContext == PC_CMV_CONF) || (currentContext == PC_CSV_CONF))
          {
            model_def_restore_vars_from_backup();
            model_deselect_current_btn(vi);
            model_restore_conf_btns(vi);
          }
          model_def_backup_vars();
          context_mgr_start_context_selection(PC_CMV_CONTEXT, PC_CMV_CONF);
          break;
        case PC_CSV_VAL:
          if((currentContext == VC_CMV_CONF) || (currentContext == PC_CMV_CONF) || (currentContext == PC_CSV_CONF))
          {
            model_def_restore_vars_from_backup();
            model_deselect_current_btn(vi);
            model_restore_conf_btns(vi);
          }
          model_def_backup_vars();
          context_mgr_start_context_selection(PC_CSV_CONTEXT, PC_CSV_CONF);
          break; 
        case BACK_VAL:
        {
          autodiag_struct ad = autodiag_mgr_get();
          if(!ad.inProgress)
          {
            if((currentContext == VC_CMV_CONF) || (currentContext == PC_CMV_CONF) || (currentContext == PC_CSV_CONF))
            {
              model_def_restore_vars_from_backup();
              model_deselect_current_btn(vi);
              model_restore_conf_btns(vi);
            }
            context_mgr_calc_back_button_operation(vi);
            vi->clearScreen_data.active = true;
            if(!context_mgr_get_pass_initial_state())
            {
              alm_mgr_deactivate_all_alarms(&(vi->alarm_data), &(vi->buzzer_data));
            }
          }
          break;
        }
        case NEXT_VAL:
        {
          context_mgr_calc_next_button_operation();
          uint8_t operationFlag = context_mgr_get_operation_set_flag();
          model_set_operation_mode(operationFlag, vi);
          if(!context_mgr_get_conf_check())
          {
            uint8_t targetContext = context_mgr_get_target_context();
            var_struct var = model_def_get_var(lastValidVarId);
            var.varValue = btnStartingVal;
            model_def_set_var(var);
            model_update_widget_array(var, &(vi->widgetBulk_data), SET);
            model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
            
            vi->widgetBulk_data.index++;

            model_calc_theoretical_ie(&(vi->widgetBulk_data));
            vi->widgetBulk_data.active = true;
          }
          else
          {
            if(model_check_if_btn_reset(lastValidVarId))
            {
              var_struct var = model_def_get_var(currentBtn.varId);
              model_def_set_var(var);
              model_update_widget_array(var, &(vi->widgetBulk_data), SET);
              model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
            
              vi->btnCtl_data.lastButtonId = currentBtn.buttonSelect;
              vi->btnCtl_data.lastButtonVal = currentBtn.offValue;

              currentBtn = model_get_btn_data(NO_SEL_VAL);

              vi->btnCtl_data.currentButtonId = currentBtn.buttonSelect;
              vi->btnCtl_data.currentButtonVal = currentBtn.offValue;

              vi->btnCtl_data.active = true;
            }
          }
          selectFlag = false;
          if((currentContext == VC_CMV_CONTEXT) || (currentContext == PC_CMV_CONTEXT) || (currentContext == PC_CSV_CONTEXT))
            vi->clearScreen_data.active = true;
          break;
        }
        case ALM_CONFIG_VAL:
        {
          Serial.print("Current context: "); Serial.println(context_mgr_get_current_context());
          if(context_mgr_get_current_context() != AUTODIAGNOSTIC)
            context_mgr_set_alarm_context();
          break; 
        }
        case VC_CMV_CONF:
        case PC_CMV_CONF:
        case PC_CSV_CONF:
        {
          model_def_backup_vars();
        }
        default:
            break;
    }
    
  
    if((currentBtn.buttonValue == VC_CMV_VAL) || (currentBtn.buttonValue == PC_CMV_VAL) || (currentBtn.buttonValue == PC_CSV_VAL))
    {
      vi->multiBtnCtl_data.active = true;
      selectFlag = false;
    }

    

    vi->screen_data.active = true;
    vi->screen_data.screenId = context_mgr_get_current_screen_id();
  }

  model_get_context_state();
}

void model_restore_conf_btns(viewInput *vi)
{
  for(int i=1; i<VAR_NUM; i++)
  {
    var_struct var = model_def_get_var(i);
    model_def_set_var(var);
    model_update_widget_array(var, &(vi->widgetBulk_data), SET);
    model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);
  }
  vi->btnCtl_data.lastButtonId = currentBtn.buttonSelect;
  vi->btnCtl_data.lastButtonVal = currentBtn.offValue;

  currentBtn = model_get_btn_data(NO_SEL_VAL);

  vi->btnCtl_data.currentButtonId = currentBtn.buttonSelect;
  vi->btnCtl_data.currentButtonVal = currentBtn.offValue;

  vi->btnCtl_data.active = true;
}

void model_toggle_pause_state(viewInput *vi)
{
  // if(context_mgr_get_pause_state())
  if(modelPauseState)
  {
    context_mgr_set_pause_state(false);
    modelPauseState = false;
    vi->clearScreen_data.active = true;
  }
  else
  {
    modelPauseState = true;
    context_mgr_set_pause_state(true);
  }
}

bool model_check_conf_values(uint8_t opMode)
{
  bool respResult;

  float ie = model_get_theoretical_ie(opMode);
  
  if((ie >= THEORETICAL_IE_MIN) && (ie <= THEORETICAL_IE_MAX))
    respResult = true;
  else
    respResult = false;

  return respResult;
}

void model_handle_encoder_cmd(modelInput* mo, viewInput* vi)
{
  autodiag_struct ad = autodiag_mgr_get();

  if(mo->encInput_data.encValue == 1){
    if((ad.o2_test) && (ad.o2_test_stage == 1) && (ad.o2_high_active))
    {
      uint16_t updateReg;

      if(ad.o2_config_val < AUTODIAG_O2_MAX)
        ad.o2_config_val++;
      autodiag_mgr_set(ad);

      updateReg = AUTO_DIAG_O2_HIGH_NUMERIC;

      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].active = true;
      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].reg = updateReg;
      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].val = ad.o2_config_val;
      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].decimal = 0;
      vi->widgetBulk_data.index++;
      vi->widgetBulk_data.active = true;
    }
    else
    {
      var_struct var = model_def_get_var(currentBtn.varId);
      if(var.varId == NOVAR)
      {
        return;
      }

      if (var.varValue < var.maxVal) 
        var.varValue += var.step;

      model_def_set_var(var);

      var = model_def_get_var(currentBtn.varId);
      //vi->widgetBulk_data = model_update_btns(var);
      model_update_widget_array(var, &(vi->widgetBulk_data), SET);
      model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);

      if((calcFlag == IE_CALC) || (calcFlag == ALL_CALC))
      {
        Serial.println("About to do IE calc");
        model_calc_theoretical_ie(&(vi->widgetBulk_data));
      }
      if((calcFlag == O2_CALC) || (calcFlag == ALL_CALC))
        model_calc_theoretical_o2(&(vi->widgetBulk_data));
    }
  }
  else if(mo->encInput_data.encValue == 2)
  {
    if((ad.o2_test) && (ad.o2_test_stage == 1) && (ad.o2_high_active))
    {
      uint16_t updateReg;

      if(ad.o2_config_val > AUTODIAG_O2_MIN)
        ad.o2_config_val--;
      autodiag_mgr_set(ad);

      updateReg = AUTO_DIAG_O2_HIGH_NUMERIC;

      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].active = true;
      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].reg = updateReg;
      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].val = ad.o2_config_val;
      vi->widgetBulk_data.widget_array[vi->widgetBulk_data.index].decimal = 0;
      vi->widgetBulk_data.index++;
      vi->widgetBulk_data.active = true;
    }
    else
    {
      var_struct var = model_def_get_var(currentBtn.varId);

      if(var.varId == NOVAR)
      {
        return;
      }

      if (var.varValue > var.minVal)
        var.varValue -= var.step;

      model_def_set_var(var);
      //Update BOTH in SET screen(graphics) and CONF screen(settings)
      model_update_widget_array(var, &(vi->widgetBulk_data), SET);
      model_update_widget_array(var, &(vi->widgetBulk_data), CONFIG);

      if((calcFlag == IE_CALC) || (calcFlag == ALL_CALC))
        model_calc_theoretical_ie(&(vi->widgetBulk_data));

      if((calcFlag == O2_CALC) || (calcFlag == ALL_CALC))
        model_calc_theoretical_o2(&(vi->widgetBulk_data));
      }
  }
  else if(mo->encInput_data.encValue == 5 && mo->encInput_data.buttonStatus)
  {
    if(encoderBtnStatus != BTN_PRESSED)
    {
      encoderBtnStatus = BTN_PRESSED;
    }
  }
  else if(mo->encInput_data.encValue == 6 && mo->encInput_data.buttonStatus)
  {
    if(encoderBtnStatus == BTN_PRESSED)
    {
      encoderBtnStatus = BTN_RELEASED;
    }
  }
}

void model_set_operation_mode(uint8_t osf, viewInput *vi)
{
  bool otherNextFlag = false;
  switch (osf)
  {
    case VC_CMV_CONTEXT:
      model_set_vc_cmv_mode(vi);
      break;
    case PC_CMV_CONTEXT:
      model_set_pc_cmv_mode(vi);
      break;
    case PC_CSV_CONTEXT:
      model_set_pc_csv_mode(vi);
      break;
    default:
      otherNextFlag = true;
      break;
  }

  if(context_mgr_get_conf_check() && !otherNextFlag)
  {
    vi->btnIconCtl_data.btnId = BACK_BUT;
    vi->btnIconCtl_data.btnVal = BACK_ICON;
    vi->btnIconCtl_data.active = true;
  }

  if(!backIconChangeFlag  && !otherNextFlag)
  {
    screen_element se = model_def_get_scr_element_by_btn_val(BACK_VAL);
    se.onValue = BACK_ICON;
    se.offValue = BACK_ICON;
    model_def_set_scr_element_by_btn_val(BACK_VAL, se);
    backIconChangeFlag = true;
  }
}

void model_set_back_icon_change_flag(bool value)
{
  backIconChangeFlag = value;
}

void model_set_vc_cmv_mode(viewInput *vi)
{
  bool checkConf = model_check_conf_values(VC_CMV_CONTEXT);
  if(checkConf)
  {
    float vt = model_def_get_var(VT).varValue;
    float fr = model_def_get_var(FR).varValue;
    float trigger = model_def_get_var(TRIGGER).varValue;
    float fio2 = model_def_get_var(FIO2).varValue;
    float tivc = model_def_get_var(TIVC).varValue;

    uint16_t data[7];
    data[0] = (uint16_t)vt;
    data[1] = (uint16_t)6*(vt/((tivc * 10) - HOLD_TIME)); //A TiVC se le quita 3 décimas por el tiempo de activación del motor
    data[2] = (uint16_t)(tivc * 10) - HOLD_TIME ;   //Tiempo de insp
    data[3] = (uint16_t)HOLD_TIME;               //Tiempo de hold
    data[4] = (uint16_t)(6000.0/fr) - (data[2] + data[3]);  //Tiempo de exp
    data[5] = (uint16_t)trigger;
    data[6] = (uint16_t)fio2;
    
    ventilationStatus = true;

    control_iface_pack(0xB2, (uint8_t *)data);
    #ifdef SD_FLAG
      sd_log_command_sent_to_atmega(0xB2, (uint8_t *)data);
    #endif  
    model_def_set_stop_flag(false);
    unsigned long timeRef = sample_mgr_get_time_ref();
    sample_mgr_store_ti_start(timeRef);
    context_mgr_set_pause_state(false);
    alm_mgr_reset_cycle_counter();
    cmd_mgr_set_test_for_alarm(true);
  }
  else
  {
    var_struct var = model_def_get_var(lastValidVarId);
    var.varValue = btnStartingVal;
    model_def_set_var(var);
    vi->widgetBulk_data = model_rst_btn_val(var, lastValBtnPressed);
  }
  selectFlag = false;
}

void model_set_pc_cmv_mode(viewInput *vi)
{
  bool checkConf = model_check_conf_values(PC_CMV_CONTEXT);
  if(checkConf)
  {
    float pc = model_def_get_var(PC).varValue;
    float ti = model_def_get_var(TI).varValue;
    float fr = model_def_get_var(FR).varValue;
    float trigger = model_def_get_var(TRIGGER).varValue;
    float fio2 = model_def_get_var(FIO2).varValue;

    uint16_t data[7];
    data[0] = (uint16_t)pc;
    data[1] = (uint16_t)ti *10;
    data[2] = (uint16_t)(6000.0/fr) - data[1];
    data[3] = (uint16_t)trigger;
    data[4] = (uint16_t)fio2;

    ventilationStatus = true;

    control_iface_pack(0xB3, (uint8_t *)data);
    #ifdef SD_FLAG
      sd_log_command_sent_to_atmega(0xB3, (uint8_t *)data);
    #endif
    model_def_set_stop_flag(false);
    context_mgr_set_pause_state(false);
    alm_mgr_reset_cycle_counter();
    unsigned long timeRef = sample_mgr_get_time_ref();
    sample_mgr_store_ti_start(timeRef);
    cmd_mgr_set_test_for_alarm(true);
  }
  else
  {
    var_struct var = model_def_get_var(lastValidVarId);
    var.varValue = btnStartingVal;
    model_def_set_var(var);
    vi->widgetBulk_data = model_rst_btn_val(var, lastValBtnPressed);
  }
  selectFlag = false;
}

void model_set_pc_csv_mode(viewInput *vi)
{
  bool checkConf = model_check_conf_values(PC_CSV_CONTEXT);
  if(checkConf)
  {
    float ps = model_def_get_var(PS).varValue;
    float peep = model_def_get_var(PEEP).varValue;
    float cycling = model_def_get_var(CYCLING).varValue;
    float trigger = model_def_get_var(TRIGGER).varValue;
    float fio2 = model_def_get_var(FIO2).varValue;
    float tapnea = model_def_get_var(TAPNEA).varValue;

    uint16_t data[7];
    data[0] = (uint16_t)ps + peep;
    data[1] = (uint16_t)cycling;
    data[2] = (uint16_t)trigger;
    data[3] = (uint16_t)fio2;
    data[4] = (uint16_t)tapnea *100;

    ventilationStatus = true;

    control_iface_pack(0xB4, (uint8_t *)data);
    #ifdef SD_FLAG
      sd_log_command_sent_to_atmega(0xB4, (uint8_t *)data);
    #endif  
    context_mgr_set_pause_state(false);
    model_def_set_stop_flag(false);
    unsigned long timeRef = sample_mgr_get_time_ref();
    sample_mgr_store_ti_start(timeRef);
    alm_mgr_reset_cycle_counter();
    cmd_mgr_set_test_for_alarm(true);
  }
  else
  {
    var_struct var = model_def_get_var(lastValidVarId);
    var.varValue = btnStartingVal;
    model_def_set_var(var);
    vi->widgetBulk_data = model_rst_btn_val(var, lastValBtnPressed); 
  }

  selectFlag = false;
}

void model_set_o2_calibration_operation(void)
{
  float vt = 600;
  float fr = 20;
  float trigger = model_def_get_var(TRIGGER).varValue;
  float fio2 = model_def_get_var(FIO2).varValue;
  float tivc = model_def_get_var(TIVC).varValue;
  
  uint16_t data[7];
  data[0] = (uint16_t)vt;
  data[1] = (uint16_t)6*(vt/((tivc * 10) - HOLD_TIME)); //A TiVC se le quita 3 décimas por el tiempo de activación del motor
  data[2] = (uint16_t)(tivc * 10) - HOLD_TIME ;   //Tiempo de insp
  data[3] = (uint16_t)HOLD_TIME;               //Tiempo de hold
  data[4] = (uint16_t)(6000.0/fr) - (data[2] + data[3]);  //Tiempo de exp
  data[5] = (uint16_t)trigger;
  data[6] = (uint16_t)fio2;
  
  ventilationStatus = true;

  control_iface_pack(0xB2, (uint8_t *)data);
  #ifdef SD_FLAG
    sd_log_command_sent_to_atmega(0xB2, (uint8_t *)data);
  #endif
  model_def_set_stop_flag(false);
  alm_mgr_reset_cycle_counter();
}

void model_set_stop(void)
{
  uint16_t stopData[1];
  stopData[0] = 0x00;
  #ifdef SD_FLAG
    sd_log_stop();
  #endif  
  control_iface_pack(0xB1, (uint8_t *)stopData);
  alm_mgr_reset_cycle_counter();

  model_def_set_stop_flag(true);
  batteryCheckWindowStart = millis();
  ventilationStatus = false;
  alm_mgr_set_alarm_cycle_detector_flag(true);
  cmd_mgr_set_test_for_alarm(false);
}

void model_set_a4_request(void)
{
  uint16_t data[1];
  data[0] = 0x00;
  control_iface_pack(0xB7, (uint8_t *)data);
}

void model_get_btn_ids(uint8_t varId, uint16_t* ids, uint8_t* idSize)
{
  uint8_t index = 0;
  screen_element scrElement;
  for(int i=0; i<ELEMENT_NUM; i++)
  {
    scrElement = model_def_get_element_by_idx(i);
    if(scrElement.varId == varId)
    {
      ids[index] = scrElement.buttonValue;
      index++;
    }
  }
  *idSize = index;
}

screen_element model_get_btn_data(uint16_t btnId)
{
  screen_element se = {0, 0, 0};
  screen_element scrElement;
  for(int i=0; i<ELEMENT_NUM; i++)
  {
    scrElement = model_def_get_element_by_idx(i);
    if(scrElement.buttonValue == btnId)
    {
      se = scrElement;
      break;
    }
  }
  
  return se;
}

widgetBulk_struct model_update_btns(var_struct var)
{
  uint16_t ids[5];
  uint8_t index;
  widgetBulk_struct wbs;

  model_get_btn_ids(currentBtn.varId, ids, &index);

  for(int i=0; i<index; i++)
  {
    wbs.widget_array[i].reg = ids[i];
    wbs.widget_array[i].val = var.varValue;
    wbs.widget_array[i].decimal = var.decimal;
  }

  wbs.index = index;
  wbs.active = true;

  return wbs;
}

widgetBulk_struct model_rst_btn_val(var_struct var, uint16_t lbp)
{
  uint16_t ids[5];
  uint8_t index;
  widgetBulk_struct wbs;

  model_get_btn_ids(var.varId, ids, &index);

  for(int i=0; i<index; i++)
  {
    wbs.widget_array[i].reg = ids[i];
    wbs.widget_array[i].val = var.varValue;
    wbs.widget_array[i].decimal = var.decimal;
  }

  wbs.index = index;
  wbs.active = true;
  return wbs;
}

widget_struct model_update_widget_value(var_struct var, uint16_t reg)
{
  widget_struct ws;

  ws.decimal = var.decimal;
  ws.reg = reg;
  ws.val = var.varValue;
  ws.active = true;

  return ws;
}

ctl_struct model_check_ctl_iface()
{
  ctl_struct temp_ctl;
  uint8_t dataBuffer[30]; //Change to 10
  uint8_t bufferIndex;
  uint8_t cmd;

  cmd = 0;

  temp_ctl.active = false;
  temp_ctl.cmd = 0;
  temp_ctl.p = 0;
  temp_ctl.f = 0;
  temp_ctl.v = 0;


  if(control_iface_check_data(dataBuffer, &bufferIndex))
  {
    cmd = control_iface_get_cmd(dataBuffer, bufferIndex);

    temp_ctl.cmd = cmd;
    temp_ctl.active = true;
  }

  switch(cmd)
  {
    case 0xA1:
    {
      pPtr = (uint8_t *)&measured_values.p;
      fPtr = (uint8_t *)&measured_values.f;
      vPtr = (uint8_t *)&measured_values.v;

      for(int i=0; i<4; i++)
      {
        *(pPtr + i) = dataBuffer[4 + i];
        *(fPtr + i) = dataBuffer[8 + i];
        *(vPtr + i) = dataBuffer[12 + i];
      }

      temp_ctl.p = measured_values.p;
      temp_ctl.f = measured_values.f;
      temp_ctl.v = measured_values.v;
      
      break;
    }
    case 0xA3:
      #ifdef CMD_A3_DEBUG_PRINT
      Serial.print("payload size: "); Serial.println(dataBuffer[2]);
      Serial.print("Command: "); Serial.println(dataBuffer[3]);
      Serial.print("Stage: "); Serial.println(dataBuffer[4]);
      Serial.print("Triggered Inspiration: "); Serial.println(dataBuffer[5]);
      #endif
      temp_ctl.stage = dataBuffer[4];
      if(dataBuffer[2] == 3)
        temp_ctl.triggeredInsp = dataBuffer[5];
      else if(dataBuffer[2] == 2)
        temp_ctl.triggeredInsp = 0;
      break;
    case 0xA4:
    {
      uint16_t temp_var;
      temp_ctl.o2 = dataBuffer[4];
      temp_var = (dataBuffer[5] << 8) | dataBuffer[6];
      temp_ctl.volt = (float)temp_var/10;
      temp_var = (dataBuffer[7] << 8) | dataBuffer[8];
      temp_ctl.i = (float)temp_var/10;
      temp_ctl.ac = dataBuffer[9];
      temp_ctl.over_pressure = dataBuffer[10];
      temp_ctl.over_flow = dataBuffer[11];
      temp_ctl.position_alarm = dataBuffer[12];
      temp_ctl.apneaTrigger = dataBuffer[13];
      temp_ctl.sensorDisconnect = dataBuffer[14];
      temp_ctl.disconnection = dataBuffer[15];
      temp_ctl.obstruction = dataBuffer[16];
      operationMode = dataBuffer[17];

      #ifdef BATTERY_PRINT
      Serial.print("Battery Value: "); Serial.println(temp_ctl.volt);
      #endif
      
      break;
    }
    case 0xA5:
    {
      uint8_t confirmationByte;
      
      confirmationByte = dataBuffer[4];
      // if(confirmationByte == 0x55)
      // {
      //   powerButtonStatus = true;
      // }
      powerButtonStatus = true;
      break;
    }
    case 0xA6:
      o2_calibration_result_code = dataBuffer[4];
      break;
    case 0xA7:
      system_status_codes = dataBuffer[4];
      break; 
    case 0xA8:   // Temporary code, must be updated to the actual Code
      {
        avrCalibrationTableFlag = true;
        //Do something to get the values from the information
        for(int i=0; i<(dataBuffer[2] + 3); i++)
          avrCalibrationTableBuffer[i] = dataBuffer[i];
        avrCalibrationTableIndex = dataBuffer[2] + 3;
        break;
      }
    default:
      break;
  }
  return temp_ctl;
}

uint8_t model_get_operation_mode(void)
{
  return operationMode;
}

bool Is_PC_congruent(uint8_t opMode){
  float pc;
  if(opMode == PC_CSV_CONTEXT)
  {
    pc = model_def_get_var(PS).varValue;
  }
  else
  {
    pc = model_def_get_var(PC).varValue;    
  }

  float peep = model_def_get_var(PEEP).varValue;

  return ((pc - peep) <= PC_PEEP_DELTA);
}

bool Is_exhalation_time_congruent(void){
  float rpm = model_def_get_var(FR).varValue;
  float ti = model_def_get_var(TI).varValue / 10.0;

  return (((60.0 / rpm)  -  ti) > MINIMUM_EXHALATION_TIME);
}

bool Is_inhalation_time_congruent(void){
  float Vt = model_def_get_var(VT).varValue;
  float flow = model_def_get_var(FLOW).varValue;

  return  ((Vt / flow)*(3.0/50.0) > MINIMUM_INHALATION_TIME);    //Factor 3/50 is for unit conversion

}

void model_update_widget_array(var_struct var, widgetBulk_struct *wbs, screen_element_type type){


  uint16_t butVal = model_def_get_scr_element_by_var(var.varId, type).buttonValue;

  wbs->widget_array[wbs->index - 1].val = var.varValue;
  wbs->widget_array[wbs->index - 1].reg = butVal;
  wbs->widget_array[wbs->index - 1].decimal = var.decimal;

  wbs->index++;
  wbs->active = true;
  
}

void model_update_widget_array_by_mvs(measuredVars_struct mvs, widgetBulk_struct *wbs){
  wbs->widget_array[wbs->index - 1].val = mvs.measuredVal;
  wbs->widget_array[wbs->index - 1].reg = mvs.buttonValue;
  wbs->widget_array[wbs->index - 1].decimal = mvs.decimal;

  wbs->index++;
  wbs->active = true;
}

void model_calc_theoretical_ie(widgetBulk_struct *wbd)
{
  measuredVars_struct mvs_c, mvs_g, mvs_c_secondary, mvs_g_secondary;
  uint8_t current_context;

  float rpm = model_def_get_var(FR).varValue;
  float tiempo_insp = 0;
  float tiempo_insp_sec = 0;

  // target_context = context_mgr_get_target_context();
  current_context = context_mgr_get_current_context();
  if(current_context == VC_CMV_CONTEXT || current_context == VC_CMV_CONF){
    tiempo_insp = model_def_get_var(TIVC).varValue;
    mvs_c = model_def_get_measured_var_by_id(C_CALCULATED_IE);
    mvs_g = model_def_get_measured_var_by_id(G_CALCULATED_IE);

    if(currentBtn.varId == FR)
    {
      mvs_c_secondary = model_def_get_measured_var_by_id(C_CALCULATED_IE_P);
      mvs_g_secondary = model_def_get_measured_var_by_id(G_CALCULATED_IE_P);
      tiempo_insp_sec = model_def_get_var(TI).varValue;
    }
  }
  else{
    tiempo_insp = model_def_get_var(TI).varValue;  
    mvs_c = model_def_get_measured_var_by_id(C_CALCULATED_IE_P);
    mvs_g = model_def_get_measured_var_by_id(G_CALCULATED_IE_P);

    if(currentBtn.varId == FR)
    {
      mvs_c_secondary = model_def_get_measured_var_by_id(C_CALCULATED_IE);
      mvs_g_secondary = model_def_get_measured_var_by_id(G_CALCULATED_IE);
      tiempo_insp_sec = model_def_get_var(TIVC).varValue;
    }
  }
  tiempo_insp = tiempo_insp / 10.0;

  mvs_c.measuredVal = tiempo_insp/((60/rpm) - tiempo_insp);
  mvs_c.measuredVal = (1.0 / mvs_c.measuredVal) + 0.05; //Se manda el valor inverso(E)
  // Subtracting 0.5 to value so it is added on the View module, this is done to compensate for the error of type casting
  // mvs_c.measuredVal -= 0.5;

  mvs_g.measuredVal = mvs_c.measuredVal;
  Serial.print("Widget to update on graph: "); Serial.println(mvs_c.buttonValue, HEX);
  Serial.print("Widget to update on conf: "); Serial.println(mvs_g.buttonValue, HEX);
  model_update_widget_array_by_mvs(mvs_c, wbd);
  model_update_widget_array_by_mvs(mvs_g, wbd);

  if(currentBtn.varId == FR)
  {
    tiempo_insp_sec = tiempo_insp_sec/10.0;
    mvs_c_secondary.measuredVal = tiempo_insp_sec/((60/rpm) - tiempo_insp_sec);
    mvs_c_secondary.measuredVal = (1.0 / mvs_c_secondary.measuredVal) + 0.05;
    // Subtracting 0.5 to value so it is added on the View module, this is done to compensate for the error of type casting
    // mvs_c_secondary.measuredVal -= 0.5;

    mvs_g_secondary.measuredVal = mvs_c_secondary.measuredVal;
    model_update_widget_array_by_mvs(mvs_c_secondary, wbd);
    model_update_widget_array_by_mvs(mvs_g_secondary, wbd);
  }

 
}

void model_calc_theoretical_o2(widgetBulk_struct *wbd){
  measuredVars_struct mvs;
  float vt = model_def_get_var(VT).varValue;
  float rpm = model_def_get_var(FR).varValue;
  float fio2 = model_def_get_var(FIO2).varValue;

  mvs = model_def_get_measured_var_by_id(C_SUGGESTED_O2_FLOW);
  mvs.measuredVal = ((vt * rpm)*((fio2/100)-0.21)) / (1000*0.79); //Factor 1000: ml-> L
  model_update_widget_array_by_mvs(mvs, wbd);
}

float model_get_theoretical_ie(uint8_t context)
{
  float ie;

  float bpm = model_def_get_var(FR).varValue;
  float inspTime = 0;
  
  if(context == VC_CMV_CONTEXT){
    inspTime = model_def_get_var(TIVC).varValue;
  }
  else{
    inspTime = model_def_get_var(TI).varValue;  
  }

  inspTime = inspTime / 10.0;


  ie = inspTime/((60/bpm) - inspTime);
  ie = 1.0 / ie; //Se manda el valor inverso(E)
  return ie;
}

void model_get_context_state(void)
{
  savedNavigator = context_mgr_get_navigator();
  savedGlobalTarget = context_mgr_get_global_target_context();
  savedPauseState = context_mgr_get_pause_state();
  savedPassInitialState = context_mgr_get_pass_initial_state();
}

void model_set_context_state(void)
{
  context_mgr_set_navigator(savedNavigator);
  context_mgr_set_global_target_context(savedGlobalTarget);
  context_mgr_set_pause_state(savedPauseState);
  context_mgr_set_pass_initial_state(savedPassInitialState);
}

bool model_check_context_integrity(void)
{
  uint8_t currentTarget;
  uint8_t currentPauseState;
  uint8_t currentPassInitialState;

  if(!model_comp_navigators())
    return false;

  currentPauseState = context_mgr_get_pause_state();
  currentTarget = context_mgr_get_global_target_context();
  currentPassInitialState = context_mgr_get_pass_initial_state();

  if(currentPauseState != savedPauseState)
    return false;
  
  if(currentTarget != savedGlobalTarget)
    return false;

  if(currentPassInitialState != savedPassInitialState)
    return false;
  
  return true;
}

bool model_comp_navigators(void)
{
  navigator_struct currentNavigator;

  currentNavigator = context_mgr_get_navigator();

  if(currentNavigator.stage != savedNavigator.stage)
    return false;
  
  if(currentNavigator.startingContext != savedNavigator.startingContext)
    return false;
  
  if(currentNavigator.targetContext != savedNavigator.targetContext)
    return false;
  
  if(currentNavigator.targetStage != savedNavigator.targetStage)
    return false;
  
  if(currentNavigator.currContext.currentContext != savedNavigator.currContext.currentContext)
    return false;
  
  if(currentNavigator.currContext.nextContext != savedNavigator.currContext.nextContext)
    return false;
  
  if(currentNavigator.currContext.previousContext != savedNavigator.currContext.previousContext)
    return false;
  
  if(currentNavigator.currContext.screenId != savedNavigator.currContext.screenId)
    return false;

  return true;
}

void model_set_min_exp_init_val(void)
{
  float peepVal = model_def_get_var(PEEP).varValue;
  sample_mgr_initialize_min_exp_pressure(peepVal);
}

void model_store_theoretical_ie(float theoretical_ie)
{ 
  measured_values.c_calculated_ie = theoretical_ie;
  measured_values.g_calculated_ie = theoretical_ie;
}