/** @file
 *
 * @brief Application entry point.
 *
 */

#include "app.h"
#include "log.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include <stdbool.h>

// Включаем низкочастотный генератор LFCLK (обязательно для app_timer!)
static void lfclk_init(void) {
  ret_code_t err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_clock_lfclk_request(NULL);
  while (!nrf_drv_clock_lfclk_is_running()) {
    // Ждём запуска генератора 32.768 кГц
  }
}

static void timer_init(void) {
  // Инициализируем модуль App Timer
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

static void gpiote_init(void) {
  ret_code_t err_code = app_timer_init();
  // Инициализируем модуль GPIOTE, если он еще не инициализирован
  if (!nrf_drv_gpiote_is_init()) {
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
  }
}

/**
 * @brief Function for application main entry.
 */
int main(void) {
  lfclk_init();
  timer_init();
  gpiote_init();
  log_init();

  app_init();

  while (true) {
    app_process();
    log_usb_process();
  }
}
