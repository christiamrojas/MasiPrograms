#include "context_mgr.h"

//currentContext, nextContext, previousContext, screenId
static context_struct context[CONTEXT_NUM] = 
{
    {VC_CMV_CONTEXT, VC_CMV_CONF, VC_CMV_CONTEXT, VC_CMV_GRAPH_SCREEN},
    {PC_CMV_CONTEXT, PC_CMV_CONF, PC_CMV_CONTEXT, PC_CMV_GRAPH_SCREEN},
    {PC_CSV_CONTEXT, PC_CSV_CONF, PC_CSV_CONTEXT, PC_CSV_GRAPH_SCREEN},
    {VC_CMV_CONF, VC_CMV_CONTEXT, VC_CMV_CONTEXT, VC_CMV_CONF_SCREEN},
    {PC_CMV_CONF, PC_CMV_CONTEXT, PC_CMV_CONTEXT, PC_CMV_CONF_SCREEN},
    {PC_CSV_CONF, PC_CSV_CONTEXT, PC_CSV_CONTEXT, PC_CSV_CONF_SCREEN},
    {VC_CMV_ALM, VC_CMV_CONTEXT, VC_CMV_CONF, VC_CMV_ALM_SCREEN},
    {PC_CMV_ALM, PC_CMV_CONTEXT, PC_CMV_CONF, PC_CMV_ALM_SCREEN},
    {PC_CSV_ALM, PC_CSV_CONTEXT, PC_CSV_CONF, PC_CSV_ALM_SCREEN},
    {VC_CMV_ERROR, VC_CMV_CONF, VC_CMV_ERROR, VC_CMV_ERROR_SCREEN},
    {PC_CMV_ERROR, PC_CMV_CONF, PC_CMV_ERROR, PC_CMV_ERROR_SCREEN},
    {PC_CSV_ERROR, PC_CSV_CONF, PC_CSV_ERROR, PC_CSV_ERROR_SCREEN},
    {AUTODIAGNOSTIC, AUTODIAGNOSTIC, AUTODIAGNOSTIC, AUTODIAGNIOSTIC_SCREEN},
    {CLOSE_CONTEXT, CLOSE_CONTEXT, CLOSE_CONTEXT, CLOSE_SCREEN}
};

static navigator_struct thisContext;

bool confCheckVar;
bool passInitialState;

bool triggerIECalc;

uint8_t operationSetFlag;   //This indicates if a opertaion should be set when certain context is changed

static bool pauseState;

uint8_t autodiagTempTargetContext;

uint8_t targetContextBeforeAutodiag;
uint8_t targetContextBeforeShutdown;
uint8_t targetStageBeforeShutdown;
uint8_t targetContextBeforeAlarms;
uint8_t targetStageBeforeAlarms;

void context_mgr_init(void)
{
    confCheckVar = false;
    thisContext.currContext = context_mgr_get_next(VC_CMV_CONF); //set this way so CV_CMV context is set as first one
    operationSetFlag = 0;
    thisContext.targetContext = VC_CMV_CONTEXT;
    thisContext.stage = CONF;

    passInitialState = false;

    pauseState = true;

    triggerIECalc = false;

    //patch variable to ensure that target context during auto diagnostic is not overwritten, should be removed once cause of 
    //overwrite is found.
    autodiagTempTargetContext = 0;
    targetContextBeforeAutodiag = thisContext.targetContext;
    targetContextBeforeShutdown = thisContext.targetContext;
    targetStageBeforeShutdown = thisContext.targetStage;
}

void context_mgr_force_init_context(uint8_t initContext)
{
    thisContext.currContext = context_mgr_get_next(initContext);
}

context_struct context_mgr_get_next(uint8_t nextContext)
{
    for(int i=0; i<CONTEXT_NUM; i++)
    {
        if(nextContext == context[i].currentContext)
        {
            return context[i];
        }
    }

    return {0,0,0};
}

context_struct context_mgr_get_context_struct(void)
{
    return thisContext.currContext;
}

bool context_mgr_check_context(uint16_t value)
{
    bool checkStatus;

    switch(value)
    {
        case VC_CMV_VAL:
        case PC_CMV_VAL:
        case PC_CSV_VAL:
        case BACK_VAL:        
        case NEXT_VAL:
        case ALM_CONFIG_VAL:
            checkStatus = true;
            break;
        default:
            checkStatus = false;
            break;
    }

    return checkStatus;
}

void context_mgr_handle_context(uint16_t value)
{
}

uint8_t context_mgr_get_error_screen(uint8_t startingContext)
{
    uint8_t error;
    switch (startingContext)
    {
        case VC_CMV_CONTEXT:
            error = VC_CMV_ERROR;
            break;    
        case PC_CMV_CONTEXT:
            error = PC_CMV_ERROR;
            break;
        case PC_CSV_CONTEXT:
            error = PC_CSV_ERROR;
            break;
        default:
            error = 0;
            break;
    }
    return error;
}

uint8_t context_mgr_get_current_screen_id(void)
{
    return thisContext.currContext.screenId;
}

uint8_t context_mgr_get_current_context(void)
{
    return thisContext.currContext.currentContext;
}

void context_mgr_start_context_selection(uint8_t targetContext, uint8_t selectionNextContext)
{
    uint8_t previous;
    previous = thisContext.currContext.currentContext;
    if(thisContext.currContext.currentContext == VC_CMV_CONTEXT || thisContext.currContext.currentContext == PC_CMV_CONTEXT || thisContext.currContext.currentContext == PC_CSV_CONTEXT)
        thisContext.startingContext = previous;
    thisContext.currContext = context_mgr_get_next(selectionNextContext);
    thisContext.targetContext = targetContext;
    thisContext.currContext.previousContext = previous;
    thisContext.stage = CONF;
    thisContext.targetStage = GRAPH;
}

void context_mgr_set_alarm_context(void)
{
    if(thisContext.currContext.currentContext != VC_CMV_ALM)
    {
        targetContextBeforeAlarms = thisContext.targetContext;
        targetStageBeforeAlarms = thisContext.targetStage;
        thisContext.targetStage = thisContext.stage;
        thisContext.targetContext = thisContext.currContext.currentContext;
    }
    
    thisContext.currContext = context_mgr_get_next(VC_CMV_ALM);
    thisContext.stage = ALM;
}

void context_mgr_set_shutdown_error_context(void)
{
    if(thisContext.currContext.currentContext != CLOSE_CONTEXT)
    {
        targetContextBeforeShutdown = thisContext.targetContext;
        targetStageBeforeShutdown = thisContext.targetStage;
        thisContext.targetStage = thisContext.stage;
        thisContext.targetContext = thisContext.currContext.currentContext;
    }

    thisContext.currContext = context_mgr_get_next(CLOSE_CONTEXT);
    thisContext.stage = CLOSE_STAGE;
}

void context_mgr_calc_back_button_operation(viewInput *vi)
{   
    switch (thisContext.stage)
    {
    case GRAPH:
        thisContext.stage = CONF;
        thisContext.currContext = context_mgr_get_next(thisContext.startingContext);  
        break;
    case CONF:
        if(passInitialState)
        {
            thisContext.stage = GRAPH;
            thisContext.currContext = context_mgr_get_next(thisContext.startingContext);
            thisContext.targetContext = thisContext.startingContext;
            triggerIECalc = true;
        }
        else
        {
            targetContextBeforeAutodiag = thisContext.targetContext;
            thisContext.targetContext = thisContext.currContext.currentContext;
            //patch variable to ensure that target context during auto diagnostic is not overwritten, should be removed once cause of 
            //overwrite is found.
            thisContext.targetStage = thisContext.stage;
            thisContext.stage = AUTODIAG;
            thisContext.currContext = context_mgr_get_next(AUTODIAGNOSTIC);
        }
        break;
    case ALM:
        // thisContext.stage = GRAPH;
        thisContext.stage = thisContext.targetStage;
        thisContext.currContext = context_mgr_get_next(thisContext.currContext.previousContext);
        break;
    case CLOSE_STAGE:
        {
            thisContext.stage = thisContext.targetStage;
            thisContext.currContext = context_mgr_get_next(thisContext.targetContext);
            thisContext.targetContext = targetContextBeforeShutdown;
            thisContext.targetStage = targetStageBeforeShutdown;
            if((thisContext.currContext.currentContext == VC_CMV_CONTEXT) || (thisContext.currContext.currentContext == PC_CMV_CONTEXT) || (thisContext.currContext.currentContext == PC_CSV_CONTEXT))
                vi->clearScreen_data.active = true;
            // operationSetFlag = 0;
        }
        break;
    case AUTODIAG:
        //patch variable to ensure that target context during auto diagnostic is not overwritten, should be removed once cause of 
        //overwrite is found.        
        thisContext.stage = thisContext.targetStage;
        thisContext.currContext = context_mgr_get_next(thisContext.targetContext);
        thisContext.targetContext = targetContextBeforeAutodiag;
        break;
    default:
        break;
    }
}

void context_mgr_calc_next_button_operation(void)
{   
    switch (thisContext.stage)
    {
        case GRAPH:
            thisContext.stage = CONF;
            thisContext.currContext = context_mgr_get_next(thisContext.currContext.nextContext);  
            operationSetFlag = 0;
            break;
        case CONF:
            if(confCheckVar)
            {
                thisContext.stage = GRAPH;
                thisContext.currContext = context_mgr_get_next(thisContext.currContext.nextContext);
                operationSetFlag = thisContext.targetContext;
                passInitialState = true;
            }
            break;
        case ALM:
            thisContext.currContext = context_mgr_get_next(thisContext.targetContext);
            thisContext.stage = thisContext.targetStage;
            thisContext.targetContext = targetContextBeforeAlarms;
            thisContext.targetStage = targetStageBeforeAlarms;
            operationSetFlag = 0;
            break;
        case CLOSE_STAGE:
            
            break;
        case ERROR:
            thisContext.stage = CONF;
            thisContext.currContext = context_mgr_get_next(thisContext.currContext.nextContext);
            operationSetFlag = 0;
            break;
        case AUTODIAG:
            thisContext.stage = thisContext.targetStage;
            thisContext.currContext = context_mgr_get_next(thisContext.targetContext);
            break;
        default:
            operationSetFlag = 0;
            break;
    }
}

uint8_t context_mgr_get_operation_set_flag(void)
{
    return operationSetFlag;
}

uint8_t context_mgr_get_target_context(void)
{
    return thisContext.targetContext;
}

void context_mgr_set_target_context(uint8_t targetContext)
{
    thisContext.targetContext = targetContext;
}

uint8_t context_mgr_get_stage(void)
{
    return thisContext.stage;
}

void context_mgr_set_stage(uint8_t stage)
{
    thisContext.stage = stage;
}

void context_mgr_set_conf_check(uint8_t error)
{
    confCheckVar = error;
}

bool context_mgr_get_pass_initial_state(void)
{
    return passInitialState;
}

void context_mgr_set_pass_initial_state(bool value)
{
    passInitialState = value;
}

void context_mgr_set_pause_state(bool value)
{
    pauseState = value;
}

bool context_mgr_get_pause_state(void)
{
    return pauseState;
}

bool context_mgr_get_trigger_ie_calc(void)
{
    return triggerIECalc;
}

void context_mgr_set_trigger_ie_calc(bool value)
{
    triggerIECalc = value;
}

bool context_mgr_get_conf_check(void)
{
    return confCheckVar;
}

navigator_struct context_mgr_get_navigator(void)
{
    return thisContext;
}

void context_mgr_set_navigator(navigator_struct navigator)
{
    thisContext = navigator;
}

uint8_t context_mgr_get_global_target_context(void)
{
    return targetContextBeforeAutodiag;
}

void context_mgr_set_global_target_context(uint8_t value)
{
    targetContextBeforeAutodiag = value;
}

uint8_t context_mgr_get_current_context_by_screen_id(uint8_t screenId)
{
    for(int i=0; i<CONTEXT_NUM; i++)
    {
        if(context[i].screenId == screenId)
        {
            return context[i].currentContext;
        }
    }

    return -1;
}

void context_mgr_set_starting_context(uint8_t startingContext)
{
    thisContext.startingContext = startingContext;
}

