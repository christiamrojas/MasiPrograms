#include "sample_mgr.h"

samples_struct sampleData;
fsamples_struct fSamples;

float vmax, vmin;

float posFlowThreshold, negFlowThreshold;

float negFlowAccum;
uint16_t negFlowCounter;

float posFlowAccum;
uint16_t posFlowCounter;

float maxNegFlow;

float maxPosFlow;

unsigned long cycleStart;
unsigned long cycleEnd;

unsigned long tiStart;
unsigned long tiEnd;
unsigned long altTiEnd;

unsigned long holdStart;
unsigned long holdEnd;

uint16_t cycleSampleCounter;

//both values are expressed in seconds
float ti_length;
float cycle_length;
float hold_length;

bool newSampleFlag;

bool tiState;
bool opFlag;

bool inspSeen;
bool insideThresholdFlag;

bool holdStartFlag;
bool holdIsZero;

bool negFlowFlag;

bool expStart;

uint8_t inspStageFlag;

int lastSampleUsed;

bool tapneaFlag;

float minExpPressure;
uint16_t inspSeenCounter;

int idxLut[4][5] = {
    {0, 7, 0, 0, 0},
    {0, 7, 3, 0, 0},
    {0, 7, 3, 0, 0},
    {0, 6, 4, 2, 0}    
};

sample_struct samplesFIFO[FIFO_SIZE];

uint16_t totalSamplesIndex;

void sample_mgr_init(void)
{
    sampleData.writeFlag = false;
    sampleData.index = 0;
    sampleData.samplesToWrite = MAX_SAMPLES_TO_SEND;
    memset(sampleData.samples, 0.0, sizeof(sample_struct));

    fSamples.sampleCounter = 0;
    fSamples.fmax = 0.0;
    memset(fSamples.samples, 0.0, F_BUFFER_SIZE * sizeof(float));
    memset(fSamples.avgSamples, 0, AVG_BUFFER_SIZE * sizeof(float));

    tiState = TI_INACTIVE;
    opFlag = DEV_STOPPED;

    cycleSampleCounter = 0;
    inspSeen = false;

    holdStartFlag = false;
    holdIsZero = false;
    insideThresholdFlag = false;

    negFlowFlag = false;
    expStart = false;

    vmax = 0.0;
    vmin = 0.0;

    negFlowAccum = 0.0;
    negFlowCounter = 0;

    posFlowAccum = 0.0;
    posFlowCounter = 0;

    posFlowThreshold = F_THRESHOLD_INSP;
    negFlowThreshold = F_THRESHOLD_EXP;

    maxNegFlow = 0.0;
    maxPosFlow = 0.0;

    newSampleFlag = false;
    tapneaFlag = false;

    lastSampleUsed = 0;

    memset(samplesFIFO, 0, sizeof(samplesFIFO));

    totalSamplesIndex = 0;

    minExpPressure = 0;
    inspSeenCounter = 0;
}

void sample_mgr_store_sample(float p, float f, float v)
{
    sampleData.samples[sampleData.index].p = p;
    sampleData.samples[sampleData.index].f = f;
    sampleData.samples[sampleData.index].v = v;

    if((sampleData.index == 3) || (sampleData.index == 7))
    {    
        #ifdef SD_FLAG
            sd_store_pfv_in_array({p, f, v});
        #endif
    }

    // if((sampleData.index == 3) || (sampleData.index == 7))
    if((sampleData.index % 4) == 0)
    {
        newSampleFlag = true;
    }

    // newSampleFlag = true;

    sampleData.index++;

    if(sampleData.index == MAX_SAMPLE_SIZE)
    {
        sampleData.writeFlag = true;
        sampleData.index = 0;
        memset(fSamples.samples, 0, sizeof(fSamples.samples));
    }

    if(inspSeen)
    {   
        if((p < minExpPressure) && (inspSeenCounter > 0))
            minExpPressure = p;
        else if(inspSeenCounter == 0)
            minExpPressure = p;
        
        inspSeenCounter++;
    }

    uint32_t currentMillis = comms_mgr_get_current_millis();

    sample_mgr_store_sample_in_fifo(p, f, v, currentMillis);

    cycleSampleCounter++;

    totalSamplesIndex++;
}

void sample_mgr_store_sample_in_fifo(float p, float f, float v, uint32_t currentMillis)
{
    for(int i=FIFO_SIZE-1; i>0; i--)
    {
        samplesFIFO[i] = samplesFIFO[i-1];
    }

    samplesFIFO[0].p = p;
    samplesFIFO[0].f = f;
    samplesFIFO[0].v = v;
    samplesFIFO[0].millisSinceStart = currentMillis;
}

bool sample_mgr_get_write_flag(void)
{
    return sampleData.writeFlag;
}

int sample_mgr_get_write_index(void)
{   
    int idx;
        
    idx = idxLut[NUM_SAMPLES - 1][sampleData.samplesToWrite];
    sampleData.samplesToWrite--;

    if(sampleData.samplesToWrite == 0)
    {
        sampleData.writeFlag = false;
        sampleData.samplesToWrite = MAX_SAMPLES_TO_SEND; //this should be handled with a constant value to reflect its use
    }

    return idx;
}

void sample_mgr_get_samples_by_idx(int idx, float *p, float *f, float *v)
{
    *p = sampleData.samples[idx].p;
    *f = sampleData.samples[idx].f;
    *v = sampleData.samples[idx].v;
}

void sample_mgr_store_fsample(float f)
{
    for(int i=F_BUFFER_SIZE-1; i > 0; i--)
    {
        fSamples.samples[i] = fSamples.samples[i-1];
    }
    fSamples.samples[0] = f;

    fSamples.sampleCounter++;
}

float sample_mgr_get_f_avg(void)
{
    float avg = 0;
    for(int i=0; i<F_BUFFER_SIZE; i++)
    {
        avg += fSamples.samples[i];
    }

    avg /= F_BUFFER_SIZE;

    return avg;
}

void sample_mgr_store_f_avg(float fAvg)
{
    for(int i=AVG_BUFFER_SIZE-1; i>0; i--)
    {
        fSamples.avgSamples[i] = fSamples.avgSamples[i-1];
    }

    fSamples.avgSamples[0] = fAvg;
}

void sample_mgr_write_f_val(float f, bool insp)
{
    float avgVal;
    sample_mgr_store_fsample(f);
    avgVal = sample_mgr_get_f_avg();
    sample_mgr_store_f_avg(avgVal);
    // if(insp)
    sample_mgr_check_max_f(f);
    if(expStart)
    {
        if(f < 0.0)
        {
            sample_mgr_accum_neg_flow(f);
            negFlowCounter++;
        }
    }
    else
    {
        sample_mgr_accum_pos_flow(f);
        posFlowCounter++;
        // if(f > 0.0)
        // {
        //     sample_mgr_accum_pos_flow(f);
        //     posFlowCounter++;
        // }
    }
}

void sample_mgr_check_max_f(float f)
{
    if(f > fSamples.fmax)
    {
        fSamples.fmax = f;
    }
}

float sample_mgr_get_max_f(void)
{
    return fSamples.fmax;
}

bool sample_mgr_check_f_threshold_insp(uint8_t filter)
{
    uint8_t compCounter = 0;
    bool checkResult = false;

    #ifdef THRESHOLD_PRINT
        Serial.print("Current Positive Threshold: "); Serial.println(posFlowThreshold);
    #endif

    if(filter == 0)
    {
        filter = 1;
    }

    for(int i=0; i<F_BUFFER_SIZE; i++)
    {   
        if(fSamples.samples[i] >= posFlowThreshold) //Before was F_THRESHOLD_INSP   
        {
            compCounter++;
        }
    }

    if (compCounter>=filter)
    {
        checkResult = true;
    }
    return checkResult;
}

bool sample_mgr_check_f_threshold_exp(uint8_t filter)
{
    uint8_t compCounter = 0;
    bool checkResult = false;

    #ifdef THRESHOLD_PRINT
        Serial.print("Current Negative Threshold: "); Serial.println(negFlowThreshold);
    #endif

    if(filter == 0)
    {
        filter = 1;
    }

    for(int i=0; i<F_BUFFER_SIZE; i++)
    {
        if(fSamples.samples[i] <= negFlowThreshold) //Before was F_THRESHOLD_EXP
        {   
            compCounter++;
        }
    }

    if (compCounter>=filter)
    {
        checkResult = true;
    }

    return checkResult;
}

unsigned long sample_mgr_get_time_ref(void)
{
    return millis();
}

void sample_mgr_store_cycle_start(unsigned long timeRef)
{
    cycleStart = timeRef;
}

void sample_mgr_store_cycle_end(unsigned long timeRef)
{
    cycleEnd = timeRef;
}

void sample_mgr_store_ti_start(unsigned long timeRef)
{
    tiStart = timeRef;
}

void sample_mgr_store_ti_end(unsigned long timeRef)
{
    tiEnd = timeRef;
}

void sample_mgr_store_alt_ti_end(unsigned long timeRef)
{
    altTiEnd = timeRef;
}

//returns value in secs
float sample_mgr_calc_cycle_len(void)
{
    float cycleLen;

    unsigned long diff = cycleEnd - cycleStart;

    cycleLen = (float)(diff) / MILLIS_FLOAT;

    return cycleLen;
}

//returns value in secs
float sample_mgr_calc_ti_len(bool altEnd)
{
    float tiLen;
    unsigned long diff;

    if(!altEnd)
    {
        diff = tiEnd - tiStart;
    }
    else
    {    
        diff = altTiEnd - tiStart;
    }
    tiLen = (float)diff/MILLIS_FLOAT;
    
    return tiLen;
}

void sample_mgr_set_ti_state(bool state)
{
    tiState = state;
}

bool sample_mgr_get_ti_state(void)
{
    return tiState;
}

void sample_mgr_calc_ti(bool altEnd)
{
    ti_length = sample_mgr_calc_ti_len(altEnd);
}

void sample_mgr_calc_cycle(void)
{
    cycle_length = sample_mgr_calc_cycle_len();
}

float sample_mgr_get_ti_len(void)
{
    return ti_length;
}

float sample_mgr_get_cycle_len(void)
{
    return cycle_length;
}

void sample_mgr_check_ti(void)
{   
    //Argument of threshold check usually should be in regards of AVG_BUFFER_SIZE definition
    if(sample_mgr_check_f_threshold_insp(AVG_BUFFER_SIZE) && !sample_mgr_get_ti_state())
    {
        uint8_t firstPosition = sample_mgr_find_first_sample_to_pos_threshold();

        sample_mgr_set_ti_state(TI_ACTIVE);
        unsigned long currentTimeRef = sample_mgr_get_time_ref();
        currentTimeRef -= firstPosition * TIME_BETWEEN_SAMPLES_MS;
        sample_mgr_store_ti_start(currentTimeRef);
        insideThresholdFlag = true;
        tapneaFlag = true;
    }
    else if(sample_mgr_check_f_threshold_exp(AVG_BUFFER_SIZE) && sample_mgr_get_ti_state())
    {   
        sample_mgr_set_insp_end(); 
    }
}

void sample_mgr_set_insp_end(void)
{
    uint8_t firstPosition = sample_mgr_find_first_sample_to_neg_threshold();
    unsigned long currentTimeRef = sample_mgr_get_time_ref();
    currentTimeRef -= firstPosition * TIME_BETWEEN_SAMPLES_MS;
    sample_mgr_store_ti_end(currentTimeRef);
    // sample_mgr_calc_ti(false);
    sample_mgr_set_ti_state(TI_INACTIVE);
    negFlowFlag = true;
    sample_mgr_set_insp_seen(true);
    vmin = vmax;
    // insideThresholdFlag = false;
}

void sample_mgr_set_op_flag(bool flagVal)
{
    opFlag = flagVal;
}

bool sample_mgr_get_op_flag(void)
{
    return opFlag;
}

void sample_mgr_set_cycle_sample_counter(uint16_t value)
{
    cycleSampleCounter = value;
}

uint16_t sample_mgr_get_cycle_sample_counter(void)
{
    return cycleSampleCounter;
}

void sample_mgr_store_hold_start(unsigned long timeRef)
{
    holdStart = timeRef;
}

void sample_mgr_store_hold_end(unsigned long timeRef)
{
    holdEnd = timeRef;
}

float sample_mgr_calc_hold_len(void)
{
    float holdLen;
    holdLen = (float)(holdEnd - holdStart)/MILLIS_FLOAT;

    return holdLen;
}

void sample_mgr_calc_hold(void)
{
    hold_length = sample_mgr_calc_hold_len();
}

float sample_mgr_get_hold_len(void)
{
    return hold_length;
}

float sample_mgr_get_recent_avg(void)
{
    return fSamples.avgSamples[AVG_BUFFER_SIZE-1];
}

bool sample_mgr_get_insp_seen(void)
{
    return inspSeen;
}

void sample_mgr_set_insp_seen(bool state)
{
    inspSeen = state;
}

void sample_mgr_set_hold_start_flag(bool state)
{
    holdStartFlag = state;
}

bool sample_mgr_get_hold_start_flag(void)
{
    return holdStartFlag;
}

void sample_mgr_set_hold_is_zero_flag(bool state)
{
    holdIsZero = state;
}

bool sample_mgr_get_hold_is_zero_flag(void)
{
    return holdIsZero;
}

bool sample_mgr_get_inside_threshold_flag(void)
{
    return insideThresholdFlag;
}

void sample_mgr_set_inside_threshold_flag(bool value)
{
    insideThresholdFlag = value;
}

void sample_mgr_set_neg_flow_flag(bool flagVal)
{
    negFlowFlag = flagVal;
}

bool sample_mgr_get_neg_flow_flag(void)
{
    return negFlowFlag;
}

void sample_mgr_check_vmax(float v)
{
    if(v > vmax)
    {
        vmax = v;
    }
}

float sample_mgr_get_vmax(void)
{
    return vmax;
}

void sample_mgr_check_vmin(float v)
{
    if(v < vmin)
    {
        vmin = v;
    }
}

float sample_mgr_get_vmin(void)
{
    return vmin;
}
void sample_mgr_reset_vmin_vmax()
{
    vmax = 0.0;
    vmin = 0.0;
}

void sample_mgr_check_start_measurement()
{
    if(!sample_mgr_get_op_flag())
    {
        sample_mgr_set_op_flag(DEV_RUNNING);
        unsigned long currentTimeRef = sample_mgr_get_time_ref();
        sample_mgr_store_cycle_start(currentTimeRef);
    }
}

void sample_mgr_set_exp_start(bool value)
{
    expStart = value;
}

bool sample_mgr_get_exp_start(void)
{
    return expStart;
}

void sample_mgr_reset_flow_vals(void)
{
    negFlowAccum = 0.0;
    negFlowCounter = 0;

    posFlowAccum = 0.0;
    posFlowCounter = 0;
}

void sample_mgr_accum_neg_flow(float f)
{
    negFlowAccum += f;
}

void sample_mgr_accum_pos_flow(float f)
{
    posFlowAccum += f;
}

float sample_mgr_get_neg_flow_avg(void)
{
    return (negFlowAccum/(float)negFlowCounter);
}

void sample_mgr_set_new_thresholds(uint8_t thresholdType)
{
    if(thresholdType == AVG)
    {
        float negFlowAvg = (negFlowAccum/(float)negFlowCounter);
        negFlowThreshold = negFlowAvg * F_THRESHOLD_EXP_FACTOR;  //double the average for the new threshold

        // posFlowThreshold = negFlowThreshold * -1;
        float posFlowAvg = (posFlowAccum/(float)posFlowCounter);
        posFlowThreshold = posFlowAvg * F_THRESHOLD_INSP_FACTOR;
    }
    else if(thresholdType == MAX)
    {
        negFlowThreshold = maxNegFlow * F_THRESHOLD_EXP_MAX_FACTOR;
        posFlowThreshold = maxPosFlow * F_THRESHOLD_EXP_MAX_FACTOR;
    }
    else if(thresholdType == NO_THRESHOLD_UPDATE)
    {
        negFlowThreshold = negFlowThreshold;
        posFlowThreshold = posFlowThreshold;
    }

    #ifdef THRESHOLD_PRINT
    Serial.println("################################");
    Serial.print("New Value for Pos Threshold: "); Serial.println(posFlowThreshold);
    Serial.print("New Value for Neg Threshold: "); Serial.println(negFlowThreshold);
    Serial.println("################################");
    #endif
}

void sample_mgr_reset_peak_flow(void){
    fSamples.fmax = 0;
}

void sample_mgr_set_new_sample_flag(bool value)
{
    newSampleFlag = value;
}

bool sample_mgr_get_new_sample_flag(void)
{
    return newSampleFlag;
}

bool sample_mgr_get_last_sample(float *p, float *f, float *v)
{
    int selectIndex;
    uint8_t sampleDistance;
    bool checkResult;

    memset(p, 0.0, sizeof(*p));
    memset(f, 0.0, sizeof(*f));
    memset(v, 0.0, sizeof(*v));

    if(sampleData.index == 0)
        selectIndex = MAX_SAMPLE_SIZE - 1;
    else
        selectIndex = sampleData.index - 1;

    if(selectIndex % MOD_SAMPLES_TO_SEND == 0)
        checkResult = true;
    else
        checkResult = false;

    sampleDistance = selectIndex - lastSampleUsed;

    //See if we missed the last sample we were supposed to send
    if(!checkResult && (sampleDistance > MOD_SAMPLES_TO_SEND))
    {
        selectIndex = lastSampleUsed + MOD_SAMPLES_TO_SEND;
        checkResult = true;
    }
    
    if(checkResult)
    {
        *p = sampleData.samples[selectIndex].p;
        *f = sampleData.samples[selectIndex].f;
        *v = sampleData.samples[selectIndex].v;

        lastSampleUsed = selectIndex;
    }

    return checkResult;
}

sample_struct sample_mgr_get_fifo_element_by_id(uint8_t idx)
{
    return samplesFIFO[idx];
}

void sample_mgr_set_tapnea_flag(bool value)
{
    tapneaFlag = value;
}

bool sample_mgr_get_tapnea_flag(void)
{
    return tapneaFlag;
}

uint16_t sample_mgr_get_total_samples_index(void)
{
    return totalSamplesIndex;
}

void sample_mgr_reset_total_samples_index(void)
{
    totalSamplesIndex = 0;
}

void sample_mgr_handle_max_neg_flow(float f)
{
    if(f < maxNegFlow)
    {
        maxNegFlow = f;
    }
}

void sample_mgr_handl_max_pos_flow(float f)
{
    if(f > maxPosFlow)
        maxPosFlow = f;
}

void sample_mgr_reset_max_neg_flow(void)
{
    maxNegFlow = 0.0;
}

uint8_t sample_mgr_find_first_sample_to_pos_threshold(void)
{
    for(int i=F_BUFFER_SIZE-1; i>0; i--)
    {
        if(fSamples.samples[i] >= posFlowThreshold)
        {
            return i;
        }
    }

    return -1;
}

uint8_t sample_mgr_find_first_sample_to_neg_threshold(void)
{
    for(int i=F_BUFFER_SIZE-1; i>0; i--)
    {
        if(fSamples.samples[i] <= negFlowThreshold)
        {
            return i;
        }
    }

    return -1;
}

float sample_mgr_get_min_exp_pressure(void)
{
    return minExpPressure;
}

void sample_mgr_initialize_min_exp_pressure(float expPressureInit)
{
    minExpPressure = expPressureInit;
}


void sample_mgr_reset_min_neg_pressure(void)
{
    // minExpPressure = 0;
    inspSeenCounter = 0;
}