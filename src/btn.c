#include "btn.h"
#include "app_error.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include <stdbool.h>

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)
#define BUTTON_PULL NRF_GPIO_PIN_PULLUP

volatile bool is_button_pressed = false;

static void button_event_handler(nrf_drv_gpiote_pin_t pin,
                          nrf_gpiote_polarity_t action) {
  if (pin == BUTTON) {
    // На платах Nordic кнопка при нажатии замыкает пин на землю (0)
    if (nrf_gpio_pin_read(BUTTON) == 0) {
      is_button_pressed = true;
    } else {
      is_button_pressed = false;
    }
  }
}

void btn_init(void) {
  // nrf_gpio_cfg_input(BUTTON, BUTTON_PULL);

  ret_code_t err_code;

  // Инициализируем модуль GPIOTE, если он еще не инициализирован
  if (!nrf_drv_gpiote_is_init()) {
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
  }

  // TOGGLE означает, что прерывание сработает и при нажатии, и при отпускании
     nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
     in_config.pull = NRF_GPIO_PIN_PULLUP; // Внутренняя подтяжка к питанию

     err_code = nrf_drv_gpiote_in_init(BUTTON, &in_config, button_event_handler);
     APP_ERROR_CHECK(err_code);

     // Включаем отслеживание событий для пина кнопки
     nrf_drv_gpiote_in_event_enable(BUTTON, true);
}

bool btn_get_state() {
  bool pin_set = nrf_gpio_pin_read(BUTTON) ? true : false;
  return (pin_set == (BUTTON_ACTIVE_STATE ? true : false));
}
