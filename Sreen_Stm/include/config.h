#ifndef CONFIG_H
#define CONFIG_H

#define BUZZER_FLAG
#define SD_FLAG 
#define DEBOUNCE_FLAG
#define USE_ALL_SAMPLES
//#define PERIODIC_SAMPLES

// Comunicacion con el ESP
#define ESP8266_COMM_PFV
#define ESP8266_COMM_MV
#define ESP8266_COMM
// Fin de comunicacion con ESP


// Forma de apagar el dispositivo, nueva o antigua
#define SHUTDOWN_SEQ_NEW

// Estos dos prints permiten ver valores de IE
// #define IE_TIME_PRINT
// #define THRESHOLD_PRINT

// #define BATTERY_PRINT
// #define PRINT_CTL_IFACE_OUTPUT
// #define PRINT_CTL_IFACE_INPUT

// #define USE_SCREEN_BUZZER
// #define SCREEN_RTC
// #define RTC_FLAG
//#define SIGFOX_FLAG

// Profiler
//#define PROFILER_MODEL
//#define PROFILER_VIEW

// #define COMMS_INPUT_PRINT
// #define BUTTON_STAGE_PRINT

// #define THRESHOLD_PRINT

// #define CMD_A3_DEBUG_PRINT

#endif