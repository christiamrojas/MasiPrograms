#include "alarm_manager.h"

uint8_t alarmCodes[ALARM_NUM] = {NO_ALARM, ALARM_PMAX_ICON_VAL, ALARM_PMIN_ICON_VAL,\
                                ALARM_VTMAX_ICON_VAL, ALARM_VTMIN_ICON_VAL, \
                                ALARM_FRMAX_ICON_VAL, ALARM_TAPNEA_ICON_VAL, \
                                ALARM_PEEP_MAX_ICON_VAL, ALARM_BATTERY_LEVEL_ICON_VAL, \
                                ALARM_AC_STATUS_ICON_VAL, ALARM_OVERPRESSURE_ICON_VAL, \
                                ALARM_OVERFLOW_ICON_VAL, ALARM_POSITION_ICON_VAL, \
                                ALARM_DISCONNECTION_ICON_VAL, ALARM_LEAK_ICON_VAL,\
                                ALARM_OCLUSION_ICON_VAL, ALARM_FIO2_ICON_VAL};

alarms_struct alms;
bool alarm_cycle_detector_flag;
uint8_t cycle_counter;

bool silentAlm;
bool buzzerFlag;

bool recordAlarmFlag;

bool stabilityFlag;

static unsigned long almWindowStart, almWindowCurrent;

void alm_mgr_init(void)
{
    alms.numActive = 0;
    alms.timerStart = 0;
    alms.timerCurrent = 0;
    alms.blinkTimerStart = 0;
    alms.blinkTimerCurrent = 0;
    alms.buzzerTimerStart = 0;
    alms.buzzerTimerCurrent = 0;
    alms.maxSize = ALARM_NUM;
    alms.active = false;
    alms.blinkFlag = false;
    alms.previousState = false;
    alms.currentId = ALM_ARRAY_START;
    for(int i=0; i<ALARM_NUM; i++)
    {
        alms.alarms[i].active = false;
        alms.alarms[i].current = false;
        alms.alarms[i].alarmId = i + 1;
        // if(i == 0 && alarmCodes[i] == 99)
        // {
        //     alms.alarms[i].alarmCode = ALARM_PMAX_ICON_VAL;
        // }
        // else
        // {
        //     alms.alarms[i].alarmCode = alarmCodes[i];   
        // }
        alms.alarms[i].alarmCode = alarmCodes[i];  
        alms.activeAlarms[i] = false;
        stabilityFlag = false;
    }

    silentAlm = false;

    alarm_cycle_detector_flag = true;
    cycle_counter = 0;

    buzzerFlag = false;
    recordAlarmFlag = true;
}

void alm_mgr_chck_alms(alarm_struct *as, buzzer_struct *bs, ledCtl_struct *lcs)
{
    if(alms.active)
    {       
        if(alms.numActive == 1)
        {   
            if(!alms.previousState)
            {
                alms.currentId = alm_mgr_get_next_active_alm(ALM_ARRAY_START);   //argumento debe ser -1
                alms.previousState = true;
            }
        }
        else if(alms.numActive > 1)
        {   
            alms.timerCurrent = millis(); //TODO: Change to something more generic
            if((alms.timerCurrent - alms.timerStart) >= ALM_MAX_TIME)
            {
                alms.currentId = alm_mgr_get_next_active_alm(alms.currentId);
                if(alms.currentId == -1)
                {
                    alms.currentId = alm_mgr_get_next_active_alm(alms.currentId);
                }
                alms.timerStart = alms.timerCurrent;                
            }
        }
        alms.blinkTimerCurrent = millis();  //TODO: Change to something more generic
        if((alms.blinkTimerCurrent - alms.blinkTimerStart) > (ALM_BLINK_PERIOD / 2))
        {
            if(!alms.blinkFlag)
            {   
                alm_mgr_set_alarm(alms.alarms[alms.currentId].alarmCode, as);
                alms.blinkFlag = true;
            }
            else
            {
                alm_mgr_set_alarm(NO_ALARM, as);
                alms.blinkFlag = false;
            }

            alms.blinkTimerStart = alms.blinkTimerCurrent;
        }


        if(!bs->active)
        {
            alms.buzzerTimerCurrent = millis(); //Should change later to a more hardware independent implementation
            if((alms.buzzerTimerCurrent - alms.buzzerTimerStart) >= BUZZER_TIME_WINDOW)
            {
                #ifdef BUZZER_FLAG
                if(!silentAlm)
                    #ifdef USE_SCREEN_BUZZER
                    bs->active = true;
                    #endif
                    buzzerFlag = true;
                #endif
                bs->buzzTime = BUZZER_TIME;
                alms.buzzerTimerStart = alms.buzzerTimerCurrent;
            }
        }

        if(silentAlm)
        {
            if(almWindowCurrent >= SILENT_ALM_TIMEOUT)
            {
                silentAlm = false;
                buzzerFlag = true; 
            }
            almWindowCurrent = millis() - almWindowStart;
        }

        lcs->active = true;
        lcs->ledColor = RED;
    }
    else
    {
        // as->active = false;
        as->alarm_id = 0;

        lcs->active = true;
        lcs->ledColor = BLUE;
    }
    return;
}

int alm_mgr_get_next_active_alm(int startIndex)
{
    int nextIndex;

    nextIndex = -1;
    for(int i=startIndex + 1; i<ALARM_NUM; i++)
    {
        if(alms.alarms[i].active)
        {
            nextIndex = i;
            break;
        }
    }    
    return nextIndex;
}

void alm_mgr_set_alarm(uint16_t alarm_id, alarm_struct *as){
    as->active = true;
    as->alarm_id = alarm_id;
}

void alm_mgr_activate(uint8_t alarmIndex)
{
    if(alarm_cycle_detector_flag && recordAlarmFlag)
    {
        if(alms.alarms[alarmIndex].active == false){
            alms.alarms[alarmIndex].active = true;
            alms.numActive++;
            alms.active = true;
            silentAlm = false;
        }
    }
}

void alm_mgr_deactivate(alarm_struct *as, buzzer_struct *bs, int alarm_id)
{
    int currentId = 0;
    if(alarm_id == -1)
        alarm_id = alms.currentId;
        
    if(alms.alarms[alarm_id].active)
    {
        if(alms.numActive == 1)
        {   
            alm_mgr_set_alarm(NO_ALARM, as);
            alms.numActive--;
            alms.active = false;
            if(alarm_id == -1)
                currentId  = alm_mgr_get_next_active_alm(ALM_ARRAY_START);
            else
                currentId = alarm_id;
            
            alms.alarms[currentId].active = false;
            alms.previousState = false;
            bs->active = false;
            buzzerFlag = false;
        }
        else if (alms.numActive > 1)
        {
            if(alarm_id == -1)
            {
                alms.alarms[alms.currentId].active = false;
            }
            else
                alms.alarms[alarm_id].active = false;
            
            alms.numActive--;
            alms.timerStart = 0;

            if(alms.numActive == 1){
                alms.previousState = false;
            }
        }
    }
}


void alm_mgr_set_silent_alm(bool value)
{
    silentAlm = value;
}

bool alm_mgr_get_silent_alm(void)
{
    return silentAlm;
}

void alm_mgr_inc_cycle_counter(void)
{
    if(cycle_counter < CYCLE_COUNTER_THRESHOLD)
    {
        cycle_counter++;
    }
    else if(cycle_counter == CYCLE_COUNTER_THRESHOLD)
    {
        alarm_cycle_detector_flag = true;
        stabilityFlag = true;
    }
}

void alm_mgr_reset_cycle_counter(void)
{
    cycle_counter = 0;
    alarm_cycle_detector_flag = false;
    stabilityFlag = false;
}

bool alm_mgr_get_stability_flag(void)
{
    return stabilityFlag;
}

uint8_t alm_mgr_get_cycle_counter(void)
{
    return cycle_counter;
}

bool alm_mgr_check_cycle_counter(uint8_t num_cycles)
{
    if(cycle_counter >= num_cycles)
        return true;
    else
        return false;
}

uint8_t alm_mgr_get_idx_by_code(uint8_t alarmCode)
{
    uint8_t alarmIdx = -1;

    for(int i=0; i<ALARM_NUM; i++)
    {       
        if(alarmCodes[i] == alarmCode)
        {
            alarmIdx = i;
            break;
        }
    }

    return alarmIdx;
} 

bool alm_mgr_get_active_status_by_code(uint8_t alarmCode)
{  
    bool status = false;

    for(int i=0; i<ALARM_NUM; i++)
    {
        if(alarmCodes[i] == alarmCode)
        {
            status = alms.alarms[i].active;
            break;
        }
    }

    return status;
}

bool alm_mgr_get_buzzer_flag(void)
{
    return buzzerFlag;
}

void alm_mgr_set_record_alarm_flag(bool value)
{
    recordAlarmFlag = value;
}

void alm_mgr_start_silent_timer(void)
{
    almWindowStart = millis();
    almWindowCurrent = 0;
}

void alm_mgr_set_alarm_cycle_detector_flag(bool value)
{
    alarm_cycle_detector_flag = value;
}

uint8_t alm_mgr_get_num_alarms_active(void)
{
    return alms.numActive;
}

void alm_mgr_deactivate_all_alarms(alarm_struct *as, buzzer_struct *bs)
{
    for(int i=0; i<ALARM_NUM; i++)
    {
        alms.alarms[i].active = false;
    }

    alm_mgr_set_alarm(NO_ALARM, as);
    alms.active = false;
    alms.numActive = 0;

    bs->active = false;
    buzzerFlag = false;
}