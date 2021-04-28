#ifndef ADDRESS_DEFINITIONS_H
#define ADDRESS_DEFINITIONS_H

//*** VALORES MEDIDOS EN PANTALLA
#define PMES_VAL            0x0100    // Presion Meseta
#define PIP_VAL             0x0101    // Peak Inspiration Pressure
#define PEEP_VAL            0x0102    // Pression end of expiration
#define FR_VAL              0x0103    // Frecuency respiratory
#define MVE_VAL             0x0105    // Volumen per minute
#define VTE_VAL             0x0106    // Volumen tidal Expiration
#define VTI_VAL             0x0107    // Volumen tidal Inspiration
#define I_VAL               0x0108    // Inpiration
#define E_VAL               0x0109    // Expiration
#define C_STATIC_VAL        0x0110
#define PEAK_FLOW_VAL       0x0111
#define FIO2_VAL            0x0112    // Oxygen
#define COLON_VAL           0x011F

//*** VALORES DE CONFIGURACION
#define WEIGHT_VAL          0x0150
#define WEIGHT_BUT          0x0250

#define FLOW_CONF_VAL       0x0152
#define FLOW_CONF_BUT       0x0252

#define FLOW_SET_VAL        0x0352 
#define FLOW_SET_BUT        0x0452

#define FR_CONF_VAL         0x0154
#define FR_CONF_BUT         0x0254

#define FR_SET_VAL          0x0354
#define FR_SET_BUT          0x0454

#define VT_CONF_VAL         0x0156
#define VT_CONF_BUT         0x0256

#define VT_SET_VAL          0x0356
#define VT_SET_BUT          0x0456

#define TRIGGER_CONF_VAL    0x0158
#define TRIGGER_CONF_BUT    0x0258

#define TRIGGER_SET_VAL     0x0358
#define TRIGGER_SET_BUT     0x0458

#define FIO2_CONF_VAL       0x015A
#define FIO2_CONF_BUT       0x025A

#define FIO2_SET_VAL        0x035A
#define FIO2_SET_BUT        0x045A

#define PEEP_CONF_VAL       0x015C
#define PEEP_CONF_BUT       0x025C

#define PEEP_SET_VAL        0x035C
#define PEEP_SET_BUT        0x045C

#define PC_CONF_VAL         0x015E
#define PC_CONF_BUT         0x025E

#define PC_SET_VAL          0x035E
#define PC_SET_BUT          0x045E

#define TIVC_CONF_VAL       0X017B
#define TIVC_CONF_BUT       0X027B

#define TIVC_SET_VAL        0x037B
#define TIVC_SET_BUT        0x047B

#define TI_SET_VAL          0x0360
#define TI_SET_BUT          0x0460

#define TI_CONF_VAL         0x0160
#define TI_CONF_BUT         0x0260

#define PS_CONF_VAL         0x0162
#define PS_CONF_BUT         0x0262

#define PS_SET_VAL          0x0362
#define PS_SET_BUT          0x0462

#define CICLYNG_CONF_VAL    0x0164
#define CICLYNG_CONF_BUT    0x0264

#define CICLYNG_SET_VAL     0x0364
#define CICLYNG_SET_BUT     0x0464

#define TAPNEA_CONF_VAL     0x0166
#define TAPNEA_CONF_BUT     0x0266

#define TAPNEA_SET_VAL      0x0366
#define TAPNEA_SET_BUT      0x0466


//*** VALORES DE ALARMA
#define ALMPMAX_VAL         0x0170
#define ALMPMAX_BUT         0x0270

#define ALMPMIN_VAL         0x0172
#define ALMPMIN_BUT         0x0272

#define ALMVTMAX_VAL        0x0174
#define ALMVTMAX_BUT        0x0274

#define ALMVTMIN_VAL        0x0176
#define ALMVTMIN_BUT        0x0276

#define ALMFRMAX_VAL        0x0178
#define ALMFRMAX_BUT        0x0278

#define ALMPEEP_VAL         0x017A
#define ALMPEEP_BUT         0x027A

#define ALMFIO2_VAL         0x017C
#define ALMFIO2_BUT         0x027C

#define SILENT_ALM_VAL      0x0FAA
#define SILENT_ALM_BUT      0x0FAA

//*** MODOS DE VENTILACION
#define VC_CMV_VAL          0x0001
#define VC_CMV_BUT          0

#define PC_CMV_VAL          0x0002
#define PC_CMV_BUT          0

#define PC_CSV_VAL          0x0003
#define PC_CSV_BUT          0

//****************************
//Context Values
//****************************
#define VC_CMV_CONTEXT      1
#define PC_CMV_CONTEXT      2
#define PC_CSV_CONTEXT      3
#define VC_CMV_CONF         4
#define PC_CMV_CONF         5
#define PC_CSV_CONF         6
#define VC_CMV_ALM          7
#define PC_CMV_ALM          8
#define PC_CSV_ALM          9
#define VC_CMV_ERROR        10
#define PC_CMV_ERROR        11
#define PC_CSV_ERROR        12

#define AUTODIAGNOSTIC      13

#define CLOSE_CONTEXT       14

//*** SET OK
#define VC_CMV_SET_VAL      0x0F11 
#define VC_CMV_SET_BUT      0

#define VC_CMV_OK_VAL       0x0F22
#define VC_CMV_OK_BUT       0

#define PC_CMV_SET_VAL      0x0F33 
#define PC_CMV_SET_BUT      0

#define PC_CMV_OK_VAL       0x0F44
#define PC_CMV_OK_BUT       0

#define PC_CSV_SET_VAL      0x0F55
#define PC_CSV_SET_BUT      0

#define PC_CSV_OK_VAL       0x0F66
#define PC_CSV_OK_BUT       0

#define ALARM_VAL           0x0F77
#define ALARM_BUT           0x0F77

#define NO_SEL_VAL          0xFFFF
#define NO_SEL_BUT          0xFFFF

#define G_SUGGESTED_O2_FLOW 0x0180      //Pantalla de graficas
#define C_SUGGESTED_O2_FLOW 0x0181      //Pantalla de configuracion

#define G_CALCULATED_IE     0x0183      //Pantalla de graficas. No mostrarlo en modo asistido   Antes 0x0184
#define C_CALCULATED_IE     0x0183      //Pantalla de configuracion. No mostrarlo en modo asistido

#define G_CALCULATED_IE_P   0x0185
#define C_CALCULATED_IE_P   0x0185

#define STOP_VAL            0x00F1
#define STOP_BUT            0x00F1

#define BACK_VAL            0x0F88
#define BACK_BUT            0x0F7F

#define NEXT_VAL            0x0F99
#define NEXT_BUT            0x0000

#define SHUTDOWN_VAL 0x0FDD

#define ALM_CONFIG_VAL      0xFACE
#define ALM_CONFIG_BUT      0xB00C

#define AUTO_DIAG_VAL       0xDEAD

#define AUTO_DIAG_LEAK_VAL  0xE001
#define AUTO_DIAG_AUD_VAL   0xE003
#define AUTO_DIAG_HME_VAL   0xE002
#define AUTO_DIAG_SYSTEM_VAL  0xE000
#define AUTO_DIAG_O2_HIGH_VAL 0xE004
#define AUTO_DIAG_O2_LOW_VAL 0xE005

#define AUTO_DIAG_LEAK_CIRCLE 0x0E11
#define AUTO_DIAG_LEAK_ICON   0x0E21

#define AUTO_DIAG_AUD_ICON    0x0E22

#define AUTO_DIAG_HME_CIRCLE  0x0E13
#define AUTO_DIAG_HME_NUMERIC 0x0E33

#define AUTO_DIAG_SYSTEM_CIRCLE 0x0E00
#define AUTO_DIAG_SYSTEM_ICON 0x0E01

#define AUTO_DIAG_O2_HIGH_CIRCLE 0x0E14
#define AUTO_DIAG_O2_HIGH_ICON 0x0E24

#define AUTO_DIAG_O2_LOW_CIRCLE 0x0E15
#define AUTO_DIAG_O2_LOW_ICON 0x0E25

#define AUTODIAG_AUD_OK       0x0EA2
#define AUTODIAG_AUD_NO_OK    0x0EB2

#define PAUSE_VAL             0x0E11

#define AUTO_DIAG_O2_HIGH_NUMERIC 0x0E34
#define AUTO_DIAG_O2_LOW_NUMERIC 0x0E35

//Screen definitions
#define VC_CMV_GRAPH_SCREEN 0x000F
#define PC_CMV_GRAPH_SCREEN 0x0011
#define PC_CSV_GRAPH_SCREEN 0x0013
#define VC_CMV_CONF_SCREEN  0x0007
#define PC_CMV_CONF_SCREEN  0x0009
#define PC_CSV_CONF_SCREEN  0x000B
#define VC_CMV_ALM_SCREEN   0x000D
#define PC_CMV_ALM_SCREEN   0x000D
#define PC_CSV_ALM_SCREEN   0x000D

#define VC_CMV_ERROR_SCREEN 0x0001
#define PC_CMV_ERROR_SCREEN 0x0001
#define PC_CSV_ERROR_SCREEN 0x0001

#define FULL_ERROR_SCREEN   0x0001

#define AUTODIAGNIOSTIC_SCREEN 0x0016

#define CLOSE_SCREEN 0x0024         // This value should change to the actual screen ID

//#################################
//Battery Information
//#################################

#define BATTERY_ICON    0x0F10

#define NO_BATTERY          44
#define BATTERY_LOW         45
#define BATTERY_MED         46
#define BATTERY_HIGH        47
#define BATTERY_FULL        48
#define BATTERY_CHARGING    49

#endif
