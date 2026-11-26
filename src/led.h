#ifndef LED_H
#define LED_H

#include <stdint.h>

#define PWM_MAX_DUTY 1000 // 100% скважности (при частоте 1 кГц)

typedef enum {
  LED_Y = 0,
  LED_R,
  LED_G,
  LED_B,
  LEDS_NUMBER // Автоматически равно 4
} led_idx_t;

void led_init(void);
void led_idx_on(uint32_t led_idx);
void led_idx_off(uint32_t led_idx);
void led_idx_set_duty(uint32_t led_idx, uint16_t duty);

#endif // !LED_H
