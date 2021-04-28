#ifndef CONTEXT_MGR_H
#define CONTEXT_MGR_H

#include "Arduino.h"
#include "address_definitions.h"
#include "global_definitions.h"

typedef struct
{
    uint8_t currentContext;
    uint8_t nextContext;
    uint8_t previousContext;
    uint8_t screenId;
}context_struct;

typedef struct
{
    uint8_t stage;
    uint8_t targetStage;
    uint8_t startingContext;
    uint8_t targetContext;
    context_struct currContext;
}navigator_struct;


#define CONTEXT_NUM         14

#define GRAPH               1
#define CONF                2
#define ALM                 3
#define ERROR               4
#define AUTODIAG            5
#define CLOSE_STAGE         6

void context_mgr_init(void);
void context_mgr_force_init_context(uint8_t initContext);
void context_mgr_handle_context(uint8_t value);
context_struct context_mgr_get_next(uint8_t currentContext);
uint8_t context_mgr_get_error_screen(uint8_t startingContext);
uint8_t context_mgr_get_current_screen_id(void);
uint8_t context_mgr_get_current_context(void);
void context_mgr_start_context_selection(uint8_t targetContext, uint8_t selectionNextContext);
void context_mgr_set_alarm_context(void);
void context_mgr_set_shutdown_error_context(void);
void context_mgr_calc_back_button_operation(viewInput *vi);
void context_mgr_calc_next_button_operation(void);
uint8_t context_mgr_get_target_context(void);
uint8_t context_mgr_get_stage(void);
void context_mgr_set_conf_check(uint8_t error);
bool context_mgr_check_context(uint16_t value);
uint8_t context_mgr_get_operation_set_flag(void);
bool context_mgr_get_pass_initial_state(void);
void context_mgr_set_pass_initial_state(bool value);
void context_mgr_set_pause_state(bool value);
bool context_mgr_get_pause_state(void);
bool context_mgr_get_trigger_ie_calc(void);
void context_mgr_set_trigger_ie_calc(bool value);
bool context_mgr_get_conf_check(void);
void context_mgr_set_target_context(uint8_t targetContext);
void context_mgr_set_stage(uint8_t stage);
navigator_struct context_mgr_get_navigator(void);
void context_mgr_set_navigator(navigator_struct navigator);
uint8_t context_mgr_get_global_target_context(void);
void context_mgr_set_global_target_context(uint8_t value);
uint8_t context_mgr_get_current_context_by_screen_id(uint8_t screenId);
context_struct context_mgr_get_context_struct(void);
void context_mgr_set_starting_context(uint8_t startingContext);
#endif