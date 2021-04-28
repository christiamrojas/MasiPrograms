#include "autodiag_mgr.h"

autodiag_struct autodiag;

void autodiag_mgr_init(void)
{
    autodiag.active = false;
    autodiag.type = NONE;
    autodiag.cycle_count = 0;
    autodiag.pPlateauIdx = 0;
    memset(autodiag.pPlateaus, 0.0, sizeof(autodiag.pPlateaus));
    autodiag.audioTestActive = false;
    autodiag.buzzerDiagStart = 0;
    memset(autodiag.PIPs, 0.0, sizeof(autodiag.PIPs));
    autodiag.PIPIdx = 0;
    autodiag.inProgress = false;
    autodiag.sysCheckDone = false;
    autodiag.o2_test = false;
    autodiag.o2_high_active = false;
    autodiag.o2_low_active = false;
    autodiag.o2_config_val = 21;
    autodiag.o2_test_stage = 0;
}

autodiag_struct autodiag_mgr_get(void)
{
    return autodiag;
}

void autodiag_mgr_set(autodiag_struct ad)
{
    autodiag = ad;
}