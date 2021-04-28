#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>
#include "model.h"
#include "global_definitions.h"
#include "screen_driver.h"
#include "encoder.h"
#include "config.h"

#define LOOP_PV_REG   0x2000
#define POINT_PV_REG  0x2100

#define PV_MAXPOINTS  200
#define PV_PIXWIDTH   180
#define PV_PIXLEFT    608
#define PV_PIXTOP     60

#define PFV_PIXWIDTH  352
#define PGRID         10
#define FGRID         20
#define VGRID         100

typedef enum
{
    SCREEN_SRAM_ADDR = 0x0014,
    TI_VC_SRAM_ADDR = 0x017B,
    VOL_SRAM_ADDR = 0x0156,
    FR_SRAM_ADDR = 0x0154,
    TRIGGER_SRAM_ADDR = 0x0158,
    PEEP_SRAM_ADDR = 0x015C,
    FIO2_SRAM_ADDR = 0x015A,
    PC_SRAM_ADDR = 0x015E,
    TI_SRAM_ADDR = 0x0160,
    PS_SRAM_ADDR = 0x0162,
    CYCLING_SRAM_ADDR = 0x0164,
    TAPNEA_SRAM_ADDR = 0x0166,
    ALMPMAX_SRAM_ADDR = 0x0170,
    ALMPMIN_SRAM_ADDR = 0x0172,
    ALMVTMAX_SRAM_ADDR = 0x0174,
    ALMVTMIN_SRAM_ADDR = 0x0176,
    ALMFRMAX_SRAM_ADDR = 0x0178,
    ALMPEEP_SRAM_ADDR = 0x017A,
    ALMFIO2_SRAM_ADDR = 0x017C,
}sram_address_defs;

void view_init(void);
int view_handler(viewInput *vi, modelInput *mo);
void view_get_values_before_reset(uint16_t *values);
uint16_t view_check_touch(void);
void view_check_screen(modelInput *mo);
void view_check_encoder(modelInput* mo);
void view_struct_handler(viewInput *vi);
void view_graph_handler(float p, float f, float v, bool cycle, bool pfv_flag);
void view_multi_button_ctl_handler(multiBtnCtl_struct multiBtnCtl_data);
void view_button_ctl_handler(btnCtl_struct btnCtl_data);
void view_widget_handler(widget_struct ws);
void view_widget_bulk_handler(widgetBulk_struct wbd);
void view_set_init_screen(void);
void view_set_screen(uint16_t screenId);
void view_pscale(float pMin, float pMax);
void view_fscale(float fMin, float fMax);
void view_vscale(float vMin, float vMax);
void view_tscale(int time_3cycles);
void view_set_value(uint16_t reg, float value, uint8_t decimal);
void view_sel_button(uint16_t reg, uint16_t val);
void view_plot_pfv(bool cycle, float cmH2O, float Lmin, float mL);
void view_plot_pv(bool cycle, float cmH2O, float mL);
void view_alarm_handler(alarm_struct as);
void view_buzzer_handler(buzzer_struct bs);
void view_button_icon_ctl_handler(uint16_t btnId, uint16_t btnVal);
void view_battery_icon_handler(uint16_t iconVal);
void view_set_screen_reset(void);
void view_clear_graph_handler(uint16_t reg);
uint16_t view_get_int_value_from_sram_read(uint16_t addr);
float view_get_float_value_from_sram_read(uint16_t addr);
void view_flush_rx_buffer(void);
#endif
