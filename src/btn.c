#include "btn.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include <stdbool.h>
#include <stddef.h>

#define DEBOUNCE_DELAY_MS      40  // Задержка от дребезга контактов
#define DOUBLE_CLICK_WINDOW_MS 350 // Максимальная пауза между двумя кликами
#define HOLD_THRESHOLD_MS      250 // Порог перехода в режим удержания

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)

static btn_event_handler_t m_event_handler = NULL;

static volatile uint8_t click_count = 0;
static volatile bool button_was_released = true;
static volatile bool is_physically_pressed = false;
static volatile bool is_held = false;

APP_TIMER_DEF(m_debounce_timer);
APP_TIMER_DEF(m_double_click_timer);
APP_TIMER_DEF(m_hold_timer);

static void notify_event(btn_event_t event) {
  if (m_event_handler != NULL) {
    m_event_handler(event);
  }
}

// Окно двойного клика истекло — значит, был только одиночный клик
static void double_click_timeout_handler(void *p_context) {
  click_count = 0;
  NRF_LOG_INFO("[BTN] Single click (ignored)");
}

// Кнопка зажата дольше 250 мс — фиксируем начало удержания
static void hold_timeout_handler(void *p_context) {
  if (is_physically_pressed && !is_held) {
    is_held = true;
    click_count = 0;
    app_timer_stop(m_double_click_timer);
    NRF_LOG_INFO("[BTN] Hold start");
    notify_event(BTN_EVENT_HOLD_START);
  }
}

static void debounce_timeout_handler(void *p_context) {
  bool is_pressed = (nrf_gpio_pin_read(BUTTON) == 0);

  if (!is_pressed) {
    // Кнопка физически отпущена — разрешаем следующее нажатие
    button_was_released = true;
    is_physically_pressed = false;
    app_timer_stop(m_hold_timer);

    if (is_held) {
      is_held = false;
      NRF_LOG_INFO("[BTN] Hold stop");
      notify_event(BTN_EVENT_HOLD_STOP);
    }
    return;
  }

  // Засчитываем клик только если кнопка была отпущена перед этим нажатием
  if (button_was_released) {
    button_was_released = false;
    is_physically_pressed = true;
    click_count++;

    app_timer_stop(m_hold_timer);
    app_timer_start(m_hold_timer, APP_TIMER_TICKS(HOLD_THRESHOLD_MS), NULL);

    if (click_count == 1) {
      app_timer_start(m_double_click_timer,
                      APP_TIMER_TICKS(DOUBLE_CLICK_WINDOW_MS), NULL);
    } else if (click_count >= 2) {
      app_timer_stop(m_double_click_timer);
      app_timer_stop(m_hold_timer);
      click_count = 0;

      NRF_LOG_INFO("[BTN] Double click!");
      notify_event(BTN_EVENT_DOUBLE_CLICK);
    }
  }
}

static void button_event_handler(nrf_drv_gpiote_pin_t pin,
                                 nrf_gpiote_polarity_t action) {
  if (pin == BUTTON) {
    app_timer_stop(m_debounce_timer);
    app_timer_start(m_debounce_timer, APP_TIMER_TICKS(DEBOUNCE_DELAY_MS), NULL);
  }
}

void btn_init(btn_event_handler_t handler) {
  ret_code_t err_code;

  m_event_handler = handler;

  err_code = app_timer_create(&m_debounce_timer, APP_TIMER_MODE_SINGLE_SHOT,
                              debounce_timeout_handler);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_double_click_timer, APP_TIMER_MODE_SINGLE_SHOT,
                              double_click_timeout_handler);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_hold_timer, APP_TIMER_MODE_SINGLE_SHOT,
                              hold_timeout_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  err_code = nrf_drv_gpiote_in_init(BUTTON, &in_config, button_event_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_event_enable(BUTTON, true);

  NRF_LOG_INFO("[BTN] Initialized on pin %d", BUTTON);
}
