#include "model_definitions.h"

//buttonValue; buttonSelect; onValue; offValue; varId;
screen_element scrElements[ELEMENT_NUM] = 
{
  {NO_SEL_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {VC_CMV_VAL, VC_CMV_BUT, 1, 2, NOVAR, SET},
  {PC_CMV_VAL, PC_CMV_BUT, 1, 2, NOVAR, SET},
  {PC_CSV_VAL, PC_CSV_BUT, 1, 2, NOVAR, SET},

  {FLOW_CONF_VAL,  FLOW_CONF_BUT, 8, 7, FLOW, CONFIG},        //Units: L/min
  {FR_CONF_VAL, FR_CONF_BUT, 2, 0, FR, CONFIG},              //Units: 1/min
  {VT_CONF_VAL,     VT_CONF_BUT, 1, 0, VT, CONFIG},  //Units: mL
  {TRIGGER_CONF_VAL, TRIGGER_CONF_BUT, 4, 0, TRIGGER, CONFIG}, //Units: L/min
  {FIO2_CONF_VAL,   FIO2_CONF_BUT, 6, 0, FIO2, CONFIG},    //Units: %  
  {PEEP_CONF_VAL, PEEP_CONF_BUT, 5, 0, PEEP, CONFIG},           //Units: cmH2O
  {PC_CONF_VAL,     PC_CONF_BUT, 7, 0, PC, CONFIG},         //Units: cmH2O        
  {TI_CONF_VAL,     TI_CONF_BUT, 3, 0, TI, CONFIG},      //Units: decimas de seg, pero se envian en centesimas de seg
  {PS_CONF_VAL,     PS_CONF_BUT, 8, 0, PS, CONFIG},      //Units: cmH2O           
  {CICLYNG_CONF_VAL, CICLYNG_CONF_BUT, 9, 0, CYCLING, CONFIG}, //Units: cmH2O
  {TAPNEA_CONF_VAL, TAPNEA_CONF_BUT, 10, 0, TAPNEA, CONFIG},  //Units: segs,pero se envian en centesimas de seg
  {TIVC_CONF_VAL, TIVC_CONF_BUT, 3, 0, TIVC, CONFIG},        //Units: decimas de seg, pero se envian en centesimas de seg

  {FLOW_SET_VAL,  FLOW_SET_BUT, 36, 35, FLOW, SET},
  {FR_SET_VAL, FR_SET_BUT, 28, 0, FR, SET},             
  {VT_SET_VAL,     VT_SET_BUT, 27, 0, VT, SET},
  {TRIGGER_SET_VAL, TRIGGER_SET_BUT, 30, 0, TRIGGER, SET},
  {FIO2_SET_VAL,   FIO2_SET_BUT, 32, 0, FIO2, SET},
  {PEEP_SET_VAL, PEEP_SET_BUT, 31, 0, PEEP, SET},
  {PC_SET_VAL,     PC_SET_BUT, 33, 0, PC, SET},
  {TI_SET_VAL,     TI_SET_BUT, 29, 0, TI, SET},
  {PS_SET_VAL,     PS_SET_BUT, 34, 0, PS, SET},
  {CICLYNG_SET_VAL, CICLYNG_SET_BUT, 35, 0, CYCLING, SET},
  {TAPNEA_SET_VAL, TAPNEA_SET_BUT, 36, 0, TAPNEA, SET},
  {TIVC_SET_VAL,  TIVC_SET_BUT, 29, 0, TIVC, SET}, 
                  
  {ALMPMAX_VAL, ALMPMAX_BUT, 37, 0, ALARM_PMAX, SET},
  {ALMPMIN_VAL, ALMPMIN_BUT, 38, 0, ALARM_PMIN, SET},
  {ALMVTMAX_VAL, ALMVTMAX_BUT, 39, 0, ALARM_VT_MAX, SET},
  {ALMVTMIN_VAL, ALMVTMIN_BUT, 40, 0, ALARM_VT_MIN, SET},
  {ALMFRMAX_VAL, ALMFRMAX_BUT, 41, 0, ALARM_FRMAX, SET},
  {ALMPEEP_VAL, ALMPEEP_BUT, 42, 0, ALARM_PEEP_DELTA, SET},
  {ALMFIO2_VAL, ALMFIO2_BUT, 43, 0, ALARM_FIO2_DELTA, SET},

  {PIP_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {PMES_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {PEEP_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {FR_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {MVE_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {VTI_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {VTE_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {I_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET},
  {E_VAL, NO_SEL_BUT, 0, 0, NOVAR, SET}, 

  {VC_CMV_SET_VAL, VC_CMV_SET_BUT, 1, 2, NOVAR, SET},
  {VC_CMV_OK_VAL,  VC_CMV_OK_BUT,  1, 2, NOVAR, SET},
  {PC_CMV_SET_VAL, PC_CMV_SET_BUT, 1, 2, NOVAR, SET},
  {PC_CMV_OK_VAL,  PC_CMV_OK_BUT,     1, 2, NOVAR, SET},
  {PC_CSV_SET_VAL, PC_CSV_SET_BUT,    1, 2, NOVAR, SET},
  {PC_CSV_OK_VAL,  PC_CSV_OK_BUT,     1, 2, NOVAR, SET},
  {C_STATIC_VAL, NO_SEL_BUT,     1, 2, NOVAR, SET},
  {ALARM_VAL, ALARM_BUT, 67, 67, NOVAR, SET},
  {STOP_VAL, STOP_BUT, 1, 2, NOVAR, SET},
  {BACK_VAL, BACK_BUT, AUTODIAGNOSTIC_ICON, AUTODIAGNOSTIC_ICON, NOVAR, SET},
  {NEXT_VAL, NEXT_BUT, 1, 2, NOVAR, SET},             //autodiagnostic => Off Value 0, BACK => On Value 51
  {SILENT_ALM_VAL, SILENT_ALM_BUT, 1, 2, NOVAR, SET},

  {AUTO_DIAG_LEAK_VAL, NO_SEL_BUT, 1, 2, NOVAR, SET},
  {AUTO_DIAG_AUD_VAL, NO_SEL_BUT, 1, 2, NOVAR, SET},
  {AUTO_DIAG_HME_VAL, NO_SEL_BUT, 1, 2, NOVAR, SET},
  {AUTO_DIAG_SYSTEM_VAL, NO_SEL_BUT, 1, 2, NOVAR, SET},
  {AUTO_DIAG_O2_HIGH_VAL, NO_SEL_BUT, 1, 2, NOVAR, SET},
  {AUTO_DIAG_O2_LOW_VAL, NO_SEL_BUT, 1, 2, NOVAR, SET},

  {AUTODIAG_AUD_OK, NO_SEL_BUT, 1, 2, NOVAR, SET},
  {AUTODIAG_AUD_NO_OK, NO_SEL_BUT, 1, 2, NOVAR, SET},

  {ALM_CONFIG_VAL, ALM_CONFIG_BUT, 1, 2, NOVAR, SET},

  {PAUSE_VAL,NO_SEL_BUT, 1, 2, NOVAR, SET},
  {SHUTDOWN_VAL, 1, 2, NOVAR, SET}
};

// buttonValue; buttonSelect; measuredVal; decimal;
measuredVars_struct measuredVars[MEASURED_VARS] = {
  {PIP_VAL, NO_SEL_BUT, 0.0, 0},
  {PMES_VAL, NO_SEL_BUT, 0.0, 0},
  {PEEP_VAL, NO_SEL_BUT, 0.0, 1},
  {FR_VAL, NO_SEL_BUT, 0.0, 0},
  {MVE_VAL, NO_SEL_BUT, 0.0, 0},
  {VTI_VAL, NO_SEL_BUT, 0.0, 0},
  {VTE_VAL, NO_SEL_BUT, 0.0, 0},
  {I_VAL, NO_SEL_BUT, 1, 0},
  {E_VAL, NO_SEL_BUT, 0.0, 1},
  {C_STATIC_VAL, NO_SEL_BUT, 0.0, 0},
  {PEAK_FLOW_VAL, NO_SEL_BUT, 0.0, 0},
  {FIO2_VAL, NO_SEL_BUT, 0.0, 0},
  {C_SUGGESTED_O2_FLOW, NO_SEL_BUT, 0.0, 1},
  {G_SUGGESTED_O2_FLOW, NO_SEL_BUT, 0.0, 0},
  {G_CALCULATED_IE, NO_SEL_BUT, 3.0, 1},
  {C_CALCULATED_IE, NO_SEL_BUT, 3.0, 1},
  {COLON_VAL, NO_SEL_BUT, 0x3A, 0},
  {G_CALCULATED_IE_P, NO_SEL_BUT, 3.0, 1},
  {C_CALCULATED_IE_P, NO_SEL_BUT, 3.0, 1},
};

//varId; varValue; defaultVal; minVal; maxVal; step; decimal;
var_struct vars[VAR_NUM] = {
  {NOVAR, 0.0, 0,  0,  0, 1, 0},
  {FLOW, 0.0, 20, 5, 60, 1, 0},
  {WEIGHT, 0.0, 60, 50, 150, 1, 0},
  {FR, 0.0, 15, 4, 35, 1, 0},
  {VT, 0.0, 400,200,800, 10, 0},
  {TRIGGER, 0.0, 4,  2, 10, 1, 0},
  {FIO2, 0.0,  21, 21, 99, 1, 0},
  {PEEP, 0.0, 0, 0, 30, 1, 0},      //debe ser con un decimal
  {PC, 0.0, 15, 10, 35, 1, 0},
  {TI, 0.0, 10, 7, 75, 1, 0},
  {PS, 0.0, 10, 5, 30, 1, 0},
  {CYCLING, 0.0, 20, 5, 40, 1, 0},
  {TAPNEA, 0.0, 15, 2, 20, 1, 0},
  {ALARM_PMAX, 0.0, 30, 20, 38, 1, 0},
  {ALARM_PMIN, 0.0, 0, 0, 30, 1, 0},
  {ALARM_VT_MAX, 0.0,700,50,800, 10, 0},
  {ALARM_VT_MIN, 0.0, 100,50,800, 10, 0},
  {ALARM_FRMAX, 0.0, 35,15,45, 1, 0},
  {ALARM_PEEP_DELTA, 0.0, 2, 0.5, 5, 0.1, 1},
  {TIVC, 0.0, 10, 7, 75, 1, 0},
  {ALARM_FIO2_DELTA, 0.0, 10, 1, 20, 1, 0},
};

varsBackup_struct varBackup[VAR_NUM];

uint16_t exception_list[EXCEPTIONS_NUM] = {
  BACK_VAL,
  NO_SEL_VAL
};

static bool stopFlag;

screen_element model_def_get_element_by_idx(int idx)
{
    return scrElements[idx];
}

var_struct model_def_get_var_by_idx(int idx)
{
    return vars[idx];
}

var_struct model_def_get_var(uint16_t varId)
{
  var_struct var;

  var = vars[0];

  for(int i=0; i< VAR_NUM; i++)
  {
    if(vars[i].varId == varId)
    {
      var = vars[i];
      break;
    }
  }

  return var;
}

float model_def_get_var_real_val(var_struct var)
{
  float realVal = var.varValue;

  for(int i=0; i<var.decimal; i++)
  {
    	realVal /= 10.0;
  }

  return realVal;
}

void model_def_set_var(var_struct var)
{
  for(int i=0; i<VAR_NUM; i++)
  {
    if(vars[i].varId == var.varId)
    {
      vars[i].varValue = var.varValue;
      break;
    }
  }
}

bool model_def_store_measuredval_value(float value, int id)
{
    bool result = false;
    // measuredVars[idx].measuredVal = value;
    for(int i=0; i<MEASURED_VARS; i++)
    {
      if(measuredVars[i].buttonValue == id)
      {
        measuredVars[i].measuredVal = value;
        result = true;
        break;
      }
    }

    return result;
}

measuredVars_struct model_def_get_measuredvar_by_idx(int idx)
{
    return measuredVars[idx];
}

measuredVars_struct model_def_get_measured_var_by_id(uint16_t varId)
{
  measuredVars_struct mvs = {0, 0 , 0, 0};
  
  for(int i=0; i<MEASURED_VARS; i++)
  {
    if(measuredVars[i].buttonValue == varId)
    {
      mvs = measuredVars[i];
      break;
    }
  }

  return mvs;
}

screen_element model_def_get_scr_element_by_var(uint16_t varId, screen_element_type type)
{
  screen_element se;
  memset(&se, 0, sizeof(se));
  
  for(int i=0; i<ELEMENT_NUM; i++)
  {
    if((scrElements[i].varId == varId) && (scrElements[i].type == type))
    {
      se = scrElements[i];
      break;
    }
  }
  
  return se;
}

screen_element model_def_get_scr_element_by_btn_val(uint16_t btnValue)
{
  screen_element se = {0, 0, 0, 0, 0};

  for(int i=0; i<ELEMENT_NUM; i++)
  {
    if(scrElements[i].buttonValue == btnValue)
    {
      se = scrElements[i];
      break;
    }
  }

  return se;
}

void model_def_set_scr_element_by_btn_val(uint16_t btnValue, screen_element se)
{
  int idx;

  idx = 0;

  for(int i=0; i<ELEMENT_NUM; i++)
  {
    if(scrElements[i].buttonValue == btnValue)
    {
      idx = i;
      break;
    }
  }

  scrElements[idx] = se;
}

bool model_def_check_exceptions(uint16_t checkId)
{
  bool result;

  result = false;

  for(int i=0; i<EXCEPTIONS_NUM; i++)
  {
    if(exception_list[i] == checkId)
    {
      result = true;
      break;
    }
  }

  return result;
}

void model_def_set_stop_flag(bool value)
{
  stopFlag = value;
}

bool model_def_get_stop_flag(void)
{
  return stopFlag;
}

void model_def_backup_vars(void)
{
  for(int i=0; i<VAR_NUM; i++)
  {
    varBackup[i].varId = vars[i].varId;
    varBackup[i].varValue = vars[i].varValue;
  }
}

void model_def_print_backup_vals(void)
{
  Serial.println("-------------------------------------------");
  for(int i=0; i<VAR_NUM; i++)
  {
    Serial.print("Variable ID: "); Serial.print(varBackup[i].varId); Serial.print(", Value: "); Serial.println(varBackup[i].varValue);
  }
  Serial.println("-------------------------------------------");
}

void model_def_restore_vars_from_backup(void)
{
  for(int i=0; i<VAR_NUM; i++)
  {
    if(vars[i].varId == varBackup[i].varId)
      vars[i].varValue = varBackup[i].varValue;
  }
}

void model_def_print_vars(void)
{
  Serial.println("-------------------------------------------");
  for(int i=0; i<VAR_NUM; i++)
  {
    Serial.print("Variable ID: "); Serial.print(vars[i].varId); Serial.print(", Value: "); Serial.println(vars[i].varValue);
  }
  Serial.println("-------------------------------------------");
}