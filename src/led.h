#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LEDS_ACTIVE_STATE 0

#define LEDS_NUMBER 4

#define LED_Y 0
#define LED_R 1
#define LED_G 2
#define LED_B 3

void led_init(void);
void led_idx_on(uint32_t led_idx);
void led_idx_off(uint32_t led_idx);

#endif // !LED_H
