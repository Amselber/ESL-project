#ifndef LEDS_H
#define LEDS_H

#include "pca10059.h"
#include <stdbool.h>
#include <stdint.h>

// Led`s
void leds_init(void);
void led_idx_on(uint32_t led_idx);
void led_idx_off(uint32_t led_idx);

// Button
void btn_init(void);
bool btn_get_state();

#endif // LEDS_H
