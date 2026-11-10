#include "btn.h"
#include "nrf_gpio.h"
#include <stdbool.h>

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)
#define BUTTON_PULL NRF_GPIO_PIN_PULLUP

void btn_init(void) { nrf_gpio_cfg_input(BUTTON, BUTTON_PULL); }

bool btn_get_state() {
  bool pin_set = nrf_gpio_pin_read(BUTTON) ? true : false;
  return (pin_set == (BUTTON_ACTIVE_STATE ? true : false));
}
