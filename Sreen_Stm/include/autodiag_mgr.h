#ifndef AUTODIAG_MGR_H
#define AUTODIAG_MGR_H

#include "Arduino.h"

#define TEST_CYCLE_NUM 3
#define P_PLATEAU_THRESHOLD 38
#define BUZZER_BEEP_TIME 2000

#define O2_CALIBRATION_21 21
#define O2_CALIBRATION_100 21

#define O2_CALIBRATION_CYCLE_NUM 1

#define AUTODIAG_O2_MAX 100
#define AUTODIAG_O2_MIN 60

#define AUTODIAG_O2_LOW_VAL 21

typedef enum{
    NONE = 0,
    LEAK,
    AUD,
    HME,
    SYSTEM,
    O2_HIGH,
    O2_LOW
}autodiag_types;

typedef struct
{
    bool active;
    uint8_t type;
    uint8_t cycle_count;
    uint8_t pPlateauIdx;
    float pPlateaus[TEST_CYCLE_NUM];
    bool audioTestActive;
    unsigned long buzzerDiagStart;
    float PIPs[TEST_CYCLE_NUM];
    uint8_t PIPIdx;
    unsigned long o2TestStart;
    bool inProgress;
    bool sysCheckDone;
    bool o2_test;
    bool o2_high_active;
    bool o2_low_active;
    uint8_t o2_config_val;
    uint8_t o2_test_stage;
}autodiag_struct;

void autodiag_mgr_init(void);
autodiag_struct autodiag_mgr_get(void);
void autodiag_mgr_set(autodiag_struct ad);
#endif