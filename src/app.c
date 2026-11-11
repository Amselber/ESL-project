#include "app.h"
#include "btn.h"
#include "led.h"
#include "nrf_delay.h"
#include <stdint.h>

#define BLINK_MS 300 // on-off led duration
#define PAUSE_MS 500 // pause between dif leds

// device ID: #6593
#define LEDS_BLINKS {6, 5, 9, 3};

static const uint8_t default_counter[LEDS_NUMBER] = LEDS_BLINKS;
static uint8_t curr_counter[LEDS_NUMBER] = LEDS_BLINKS;
static uint8_t curr_idx = 0;

/**
 * @brief Blink a single LED a given number of times.
 *
 * @param[in] led_idx Order number for led
 */
static void blink_led(uint32_t led_idx) {
  led_idx_on(led_idx);
  nrf_delay_ms(BLINK_MS);
  led_idx_off(led_idx);
}

/**
 * @brief Application init
 * run before app_proccess()
 */
void app_init(void) {
  led_init();
  btn_init();
}

/**
 * @brief Application process
 * run in main loop
 */
void app_process(void) {
  if (btn_get_state()) {
    // !uncomment for optional task
    led_idx_off(curr_idx);
    if (curr_idx < LEDS_NUMBER) {
      if (curr_counter[curr_idx] > 0) {
        blink_led(curr_idx);
        --curr_counter[curr_idx];
      } else {
        curr_counter[curr_idx] = default_counter[curr_idx];
        ++curr_idx;
      }
    } else {
      curr_idx = 0;
    }
  } else {
    // !uncomment for optional task
    led_idx_on(curr_idx);
  }
  nrf_delay_ms(PAUSE_MS);
}
