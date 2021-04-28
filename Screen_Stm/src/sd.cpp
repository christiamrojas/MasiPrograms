#include <SPI.h>
#include <stdio.h>
#include "SdFat.h"
#include "screen_driver.h"
#include "utils.h"
#include "rtc.h"
#include "sd.h"

const char *LOG_PMAX_ALARM = "Alarma de Presion Maxima\n";
const char *LOG_PMIN_ALARM = "Alarma de Presion Minima\n";
const char *LOG_VTMAX_ALARM= "Alarma de Volumen Maxima\n";
const char *LOG_VTMIN_ALARM  = "Alarma de Volumen Minima\n";
const char *LOG_FR_ALARM = "Alarma de RPM\n";
const char *LOG_PEEP_ALARM = "Alarma de PEEP\n";
const char *LOG_STOP_PRESSED = "Stop\n";


SdFat sd1;
const uint8_t SD1_CS = PA4;  // chip select for sd1
char buf[SD_BUF_SIZE] = {'\0'};
pfv_log_struct pfv_buffer[SD_PFV_BUF_SIZE]={0};
int pfv_buf_idx = 0;
bool sd_first_time_on = true;

// Directory file.
SdFile root;

//------------------------------------------------------------------------------
// print error msg, any SD error codes, and halt.
// store messages in flash
#define errorExit(msg) errorHalt(F(msg))
#define initError(msg) initErrorHalt(F(msg))
//------------------------------------------------------------------------------

void sd_init(void){
    // initialize the first card
    if (!sd1.begin(SD1_CS, SD_SCK_MHZ(20))) {
        Serial.println(F("No se pudo inicializar SD"));
    }
    
    Serial.println(F("------sd1 root-------"));
    sd1.ls();
}

 void sd_remove_old_files(void){
    char filenames_not_to_be_removed[50][15];
    char filename[20] = {'\0'};
    rtc_struct rtc_time;
    for(int idx=0; idx < 50; idx++)
        memset(filenames_not_to_be_removed[idx], '\0', sizeof(filenames_not_to_be_removed[idx]));

     #ifdef RTC_FLAG
        rtc_get_timestamp(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
    #else 
    #ifdef SCREEN_RTC
        assign_screen_timestamp_to_filename(filename);
    #else
        rtc_get_timestamp_aux(&rtc_time);
        generate_untouched_file_names(&rtc_time, filenames_not_to_be_removed);
    #endif
    #endif

    SdFile file;
    if (!root.open("/"))
        Serial.println("open root failed");

    Serial.println("Borrando archivos...");
    while (file.openNext(&root, O_RDONLY)){
        if (!file.isHidden())
            file.getName(filename, sizeof(filename));
        file.close();

        //Funcion que evalúe si file)name se encuentra en el arreglo de los archivos que no se deben borrar
        if(Is_filename_in_array(filename, filenames_not_to_be_removed) == false){
            Serial.print("Filename: "); Serial.println(filename);
            sd1.remove(filename);
        }
    }

    root.close();
    Serial.println("Archivos borrados!");

 }

void sd_store_pfv_in_array(pfv_log_struct pfv){
    if(pfv_buf_idx == SD_PFV_BUF_SIZE){
        return;
    }

    pfv_buffer[pfv_buf_idx++] = pfv;
}

int sd_get_pfv_array_index(void){
    return pfv_buf_idx;
}

void sd_log_pfv(void){
    SdFile file1;
    char filename[20] = {'\0'};
    // char timestamp_str[16] = {'\0'};
    rtc_struct rtc_time;
    char str_temp[6] = {'\0'};
    rtc_time.year = 20;
    rtc_time.month = 5;
    rtc_time.day = 5;
    rtc_time.hour = 11;
    rtc_time.minute = 30;
    rtc_time.second = 0;

    #ifdef RTC_FLAG
        rtc_get_timestamp(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
    #else 
    #ifdef SCREEN_RTC
        assign_screen_timestamp_to_filename(filename);
    #else
        rtc_get_timestamp_aux(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
    #endif
    #endif

    if(sd_first_time_on){
        sd_first_time_on = false;
        sd_remove_old_files();
    }

    uint32_t start_time = millis();
    
    if (!file1.open(filename, O_RDWR | O_CREAT | O_APPEND)){
        return;
    }

    for(int i=0; i < pfv_buf_idx; i++){
        memset(buf, '\0', sizeof(buf));
        #ifdef RTC_FLAG
        rtc_get_timestamp(&rtc_time);
        add_timestamp_to_log_string(&rtc_time, buf);
        #else
        #ifdef SCREEN_RTC
        strcat(buf, timestamp_str);
        #else
        rtc_get_timestamp_aux(&rtc_time);
        add_timestamp_to_log_string(&rtc_time, buf);
        #endif
        #endif

        memset(str_temp, '\0', sizeof(str_temp));
        strcat(buf, ",p:");
        float_to_str(pfv_buffer[i].p, str_temp);
        strcat(buf, str_temp);

        memset(str_temp, '\0', sizeof(str_temp));
        strcat(buf, ",f:");
        float_to_str(pfv_buffer[i].f, str_temp);
        strcat(buf, str_temp);

        memset(str_temp, '\0', sizeof(str_temp));
        strcat(buf, ",v:");
        float_to_str(pfv_buffer[i].v, str_temp);
        strcat(buf, str_temp);
        strcat(buf, "\n");
        file1.write(buf);
    }

    pfv_buf_idx = 0;
    file1.close();
    uint32_t end_time = millis();
}

void sd_log_cyclical_variables(cycle_log_struct vars){
    char filename[20] = {'\0'};
    rtc_struct rtc_time;
    char str_temp[6] = {'\0'};
    memset(buf, '\0', sizeof(buf));
    rtc_time.year = 20;
    rtc_time.month = 5;
    rtc_time.day = 5;
    rtc_time.hour = 11;
    rtc_time.minute = 30;
    rtc_time.second = 0;

    #ifdef RTC_FLAG
    rtc_get_timestamp(&rtc_time);
    assign_timestamp_to_filename(&rtc_time, filename);
    add_timestamp_to_log_string(&rtc_time, buf);
    #else
    #ifdef SCREEN_RTC
    assign_screen_timestamp_to_filename(filename);
    add_screen_timestamp_to_log_string(buf);
    #else
    rtc_get_timestamp_aux(&rtc_time);
    assign_timestamp_to_filename(&rtc_time, filename);
    add_timestamp_to_log_string(&rtc_time, buf);
    #endif
    #endif

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",peep:");
    float_to_str(vars.peep, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",pip:");
    float_to_str(vars.pip, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",I/E:");
    float_to_str(vars.i_e, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",p_meseta:");
    float_to_str(vars.p_meseta, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",c_estatico:");
    float_to_str(vars.c_estatico, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",f_peak:");
    float_to_str(vars.f_peak, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",f_resp:");
    float_to_str(vars.f_resp, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",Vt:");
    float_to_str(vars.Vt, str_temp);
    strcat(buf, str_temp);

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",MVe:");
    float_to_str(vars.MVe, str_temp);
    strcat(buf, str_temp);
    strcat(buf, "\n");

    sd_log_new_line(buf, filename);
}

void sd_log_alarm(const char *alarm_log_str){
    char filename[20] = {'\0'};
    rtc_struct rtc_time;
    char str_temp[6] = {'\0'};

    memset(buf, '\0', sizeof(buf));
    rtc_time.year = 20;
    rtc_time.month = 5;
    rtc_time.day = 5;
    rtc_time.hour = 11;
    rtc_time.minute = 30;
    rtc_time.second = 0;

    #ifdef RTC_FLAG
    rtc_get_timestamp(&rtc_time);
    assign_timestamp_to_filename(&rtc_time, filename);
    add_timestamp_to_log_string(&rtc_time, buf);
    #else
    #ifdef SCREEN_RTC
    assign_screen_timestamp_to_filename(filename);
    add_screen_timestamp_to_log_string(buf);
    #else
    rtc_get_timestamp_aux(&rtc_time);
    assign_timestamp_to_filename(&rtc_time, filename);
    add_timestamp_to_log_string(&rtc_time, buf);
    #endif
    #endif

    memset(str_temp, '\0', sizeof(str_temp));
    strcat(buf, ",");
    strcat(buf, alarm_log_str);

    sd_log_new_line(buf, filename);
}

void sd_log_command_sent_to_atmega(uint8_t cmd, uint8_t *dataFrame){
    char filename[20] = {'\0'};
    rtc_struct rtc_time;
    char str_temp[6] = {'\0'};

    memset(buf, '\0', sizeof(buf));
    memset(str_temp, '\0', sizeof(str_temp));

    #ifdef RTC_FLAG
        rtc_get_timestamp(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
        add_timestamp_to_log_string(&rtc_time, buf);
    #else
        #ifdef SCREEN_RTC
        assign_screen_timestamp_to_filename(filename);
        add_screen_timestamp_to_log_string(buf);
        #else
        rtc_get_timestamp_aux(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
        add_timestamp_to_log_string(&rtc_time, buf);
        #endif
    #endif

    strcat(buf, ",SET: ");
    int_to_str(cmd, 3, str_temp);
    strcat(buf, str_temp);
    strcat(buf, " ");

    for(int idx=0; idx < 14; idx++){
        memset(str_temp, '\0', sizeof(str_temp));
        int_to_str(dataFrame[idx], 3, str_temp);
        strcat(buf, str_temp);
        strcat(buf, " ");
    }
    strcat(buf, "\n");
    Serial.println("Buf vale(COMMAND SENT TO ATMEGA):");
    Serial.println(buf);

    sd_log_new_line(buf, filename);
}

void sd_log_new_line(char *new_line, char *filename){
    SdFile file1;

    if (!file1.open(filename, O_RDWR | O_CREAT | O_APPEND)) {
        Serial.println("Error al abrir file1");
        return;
    }

    file1.write(new_line);
    file1.close();
}

void sd_log_stop(void){
    sd_log_alarm(LOG_STOP_PRESSED);
}

void sd_log_reset_cause(uint32_t csr_value){
    char filename[20] = {'\0'};
    char str_temp[11] = {'\0'};
    rtc_struct rtc_time;
    memset(buf, '\0', sizeof(buf));
    memset(str_temp, '\0', sizeof(str_temp));

    #ifdef RTC_FLAG
        rtc_get_timestamp(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
        add_timestamp_to_log_string(&rtc_time, buf);
    #else
        #ifdef SCREEN_RTC
        assign_screen_timestamp_to_filename(filename);
        add_screen_timestamp_to_log_string(buf);
        #else
        rtc_get_timestamp_aux(&rtc_time);
        assign_timestamp_to_filename(&rtc_time, filename);
        sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d, reset cause: 0x%08X\n", rtc_time.year, rtc_time.month,
                rtc_time.day, rtc_time.hour, rtc_time.minute, rtc_time.second, csr_value);
        
        Serial.println("Lo que se va a loggear como reset cause:");
        Serial.println(buf);
        sd_log_new_line(buf, filename);
        #endif
    #endif
}