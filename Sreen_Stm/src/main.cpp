#include <Arduino.h>
#include <RTClock.h>
#include "main_definitions.h"
#include "sd.h"
#include <libmaple/iwdg.h>
#include <libmaple/rcc.h>

uint8_t current_state = VIEW;
uint8_t next_state    = VIEW;

viewInput testView;
modelInput testModel;

viewInput *view_input;
modelInput *model_input;

uint32_t watchdog_start_time = 0;

void setup() {
  // put your setup code here, to run once:
  delay(500);

  Serial.begin(115200);
  Serial.println("Hello There!");
  delay(500);
  Serial.println("General Kenobi!");
  uint32_t csr_value = RCC_BASE->CSR;
  if(csr_value & RCC_CSR_IWDGRSTF){
    //TODO Aqui Falta
    Serial.println("Detecte reset por watchdog");
  }

  //Limpiamos reset flags
  RCC_BASE->CSR |= RCC_CSR_RMVF;

  Serial.print("Nuevos valores de reset flags en HEX: "); Serial.println(RCC_BASE->CSR, HEX);

  view_init();
  model_init();

  model_send_buzzer_cmd(ALM_BUZZER_DISABLED);

  model_set_a4_request();
  delay(20);
  model_handler(&testModel, &testView);
  model_handler(&testModel, &testView);
  model_handler(&testModel, &testView);

  uint8_t opMode = model_get_operation_mode();

  // opMode = 0;

  if(opMode == 0)
    view_set_screen_reset();
  #ifdef SIGFOX_FLAG
    sigfox_init();
  #endif
  Serial.println("model and view configurations complete, configuring SD");
  //Delay to allow the screen time after reset before sending commands to it.
  delay(1000);
  #ifdef RTC_FLAG
    rtc_set_time();
    rtc_struct rtc_time;
    rtc_get_timestamp(&rtc_time);
    Serial.println("Datos del RTC interno: ");
    Serial.println(rtc_time.year);
    Serial.println(rtc_time.month);
    Serial.println(rtc_time.day);
    Serial.println(rtc_time.hour);
    Serial.println(rtc_time.minute);
    Serial.println(rtc_time.second);
  #else
    #ifndef SCREEN_RTC
      rtc_set_time();
    #endif
  #endif
  #ifdef SD_FLAG
  sd_init();
  Serial.println("SD Setup finished loading reset cause");
  sd_log_reset_cause(csr_value);
  #endif
  comms_mgr_sync_time();
  testView.ctl_data.active = false;
  testView.ctl_data.p = 0;
  testView.ctl_data.f = 0;
  testView.ctl_data.v = 0;

  view_input = &testView;

  Serial.print("Current State: "); Serial.println(current_state);
  Serial.print("Operation Mode: "); Serial.println(opMode);

  if(opMode == 0)
  {
    Serial.println("Opmode 0, doing regular init");
    model_get_all_elements(&testView);
    view_handler(&testView, &testModel);
  }
  else
  {
    view_flush_rx_buffer();
    uint16_t values[20];
    view_get_values_before_reset(values);
    context_mgr_set_pass_initial_state(true);
    uint8_t currentContext = context_mgr_get_current_context_by_screen_id(values[0]);

    context_mgr_force_init_context(currentContext);
    
    context_struct cs = context_mgr_get_context_struct();
    
    context_mgr_set_target_context(cs.nextContext);
    context_mgr_set_global_target_context(cs.nextContext);

    if((currentContext == VC_CMV_CONTEXT) || (currentContext == PC_CMV_CONTEXT) || (currentContext == PC_CSV_CONTEXT))
      context_mgr_set_stage(GRAPH);
    else
      context_mgr_set_stage(CONF);
    
    context_mgr_set_starting_context(opMode);

    cmd_mgr_set_test_for_alarm(true);

    context_mgr_set_pause_state(false);
    model_get_context_state();

    Serial.print("Current Context is: "); Serial.println(cs.currentContext);
    Serial.print("Next Context should be: "); Serial.println(cs.nextContext);
    Serial.print("Previous Context was: "); Serial.println(cs.previousContext);
    Serial.print("Screen ID for this context is: "); Serial.println(cs.screenId);

    testView.clearScreen_data.active = true;

    model_restore_reset_values(&testView, values);
    model_set_min_exp_init_val();
    alm_mgr_reset_cycle_counter();
    view_handler(&testView, &testModel);
  }

  // model_get_all_elements(&testView);
  // view_handler(&testView, &testModel);
  model_set_off_values(view_input);
  // view_set_init_screen();
  if(opMode == 0)
  {
    view_input->screen_data.active = 1;
    view_input->screen_data.screenId = 7;
  }
  view_input->multiBtnCtl_data.active = true;
  view_input->alarm_data.alarm_id = NO_ALARM;
  view_input->alarm_data.active = true;
  
  view_input->btnIconCtl_data.btnId = BACK_BUT;
  view_input->btnIconCtl_data.btnVal = AUTODIAGNOSTIC_ICON;
  view_input->btnIconCtl_data.active = true;

  if(opMode == 0)
  {
    measuredVars_struct ie = model_def_get_measured_var_by_id(C_CALCULATED_IE);
    model_update_widget_array_by_mvs(ie, &(view_input->widgetBulk_data));
    ie =  model_def_get_measured_var_by_id(G_CALCULATED_IE);
    model_update_widget_array_by_mvs(ie, &(view_input->widgetBulk_data));

    ie =  model_def_get_measured_var_by_id(C_CALCULATED_IE_P);
    model_update_widget_array_by_mvs(ie, &(view_input->widgetBulk_data));

    ie =  model_def_get_measured_var_by_id(G_CALCULATED_IE_P);
    model_update_widget_array_by_mvs(ie, &(view_input->widgetBulk_data));

    measuredVars_struct i = model_def_get_measured_var_by_id(I_VAL);
    model_update_widget_array_by_mvs(i, &(view_input->widgetBulk_data));
  }

  //TODO: Define screen position for Reset flag

  if(opMode != 0)
  {
    screen_element se = model_def_get_scr_element_by_btn_val(BACK_VAL);
    se.onValue = BACK_ICON;
    se.offValue = BACK_ICON;
    model_def_set_scr_element_by_btn_val(BACK_VAL, se);
    model_set_back_icon_change_flag(true);

    testView.btnIconCtl_data.btnId = BACK_BUT;
    testView.btnIconCtl_data.btnVal = BACK_ICON;
    testView.btnIconCtl_data.active = true;
  }

  current_state = VIEW;

  iwdg_init(IWDG_PRE_256, 0x03FF);
  Serial.println("Setup finished");
}

void loop() 
{
  switch(current_state)
  {
    case VIEW:
      next_state = view_handler(&testView, &testModel);
      break;
    case MODEL:
      next_state = model_handler(&testModel, &testView);
      break;
  }

  current_state = next_state;
  iwdg_feed();
}