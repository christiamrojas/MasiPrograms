#ifndef SD_H
#define SD_H
#include "rtc.h"
#include "config.h"

#define SD_BUF_SIZE 150
#define SD_PFV_BUF_SIZE  30 //250 

extern const char *LOG_PMAX_ALARM;
extern const char *LOG_PMIN_ALARM;
extern const char *LOG_VTMAX_ALARM;
extern const char *LOG_VTMIN_ALARM;
extern const char *LOG_FR_ALARM;
extern const char *LOG_PEEP_ALARM;
extern const char *LOG_STOP_PRESSED;

typedef struct{
    float p;
    float f;
    float v;
}pfv_log_struct;

typedef struct{
    float peep;
    float pip;
    float p_meseta;
    float c_estatico;
    float i_e;
    float f_peak;
    float f_resp;
    float Vt;
    float MVe;
}cycle_log_struct;

void sd_init(void);
void sd_remove_old_files(void);
void sd_write_line(void);
void sd_log_pfv(void);
void sd_log_alarm(const char *alarm_log_str);
void sd_log_new_line(char *new_line, char *filename);
void sd_store_pfv_in_array(pfv_log_struct pfv);
int sd_get_pfv_array_index(void);
void sd_log_stop(void);
void sd_log_reset_cause(uint32_t csr_value);

#endif