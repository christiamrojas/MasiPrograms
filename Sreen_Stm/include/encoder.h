#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "Arduino.h"
#include "config.h"
#include "global_definitions.h"

#define enc_A       PB6   // encoder A
#define enc_B       PB5   // encoder B
#define enc_SW      PB7  // encoder button

#define LED_RED     PB4
#define LED_GREEN   PB3
#define LED_BLUE    PA15

#define enc_deboucing_ms 100      // encoder debouncing 10ms
#define enc_but_deboucing_ms 1  // encoder button debouncing 10ms
#define enc_fast_ms 50          // previous value = 50

// Hardware-specific helper functions
uint16_t enc_digital_read(uint16_t pin);

void enc_init();
int enc_read();
int enc_wheel_read();
bool enc_but_state();
int enc_but_read();
int enc_enc_read();
int enc_on_but_read();
int enc_simple_read();
void enc_set_led_color(uint8_t color);
#endif
