#include "btn.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_drv_gpiote.h"
#include <stdbool.h>

#define DEBOUNCE_DELAY_MS 40       // Задержка от дребезга контактов
#define DOUBLE_CLICK_WINDOW_MS 350 // Максимальная пауза между двумя кликами

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)

// Флаг состояния: false = пауза (заморожено), true = мигает
volatile bool g_isBlinkingActive = false;

static uint8_t click_count = 0;

// Создаём ID для двух одноразовых таймеров
APP_TIMER_DEF(m_debounce_timer);
APP_TIMER_DEF(m_double_click_timer);

// Таймер окна двойного клика истёк -> второго нажатия не было, сбрасываем
// счётчик
static void double_click_timeout_handler(void *p_context) { click_count = 0; }

// Таймер антидребезга истёк -> проверяем реальное состояние кнопки
static void debounce_timeout_handler(void *p_context) {
  // Если через 40 мс кнопка всё ещё физически нажата (уровень 0)
  if (nrf_gpio_pin_read(BUTTON) == 0) {
    click_count++;

    if (click_count == 1) {
      // Первый клик: запускаем окно ожидания второго клика (350 мс)
      app_timer_start(m_double_click_timer,
                      APP_TIMER_TICKS(DOUBLE_CLICK_WINDOW_MS), NULL);
    } else if (click_count == 2) {
      // Второй клик успел вовремя -> фиксируем DOUBLE CLICK!
      app_timer_stop(m_double_click_timer);
      click_count = 0;

      // Переключаем состояние мигания (запуск / заморозка)
      g_isBlinkingActive = !g_isBlinkingActive;
    }
  }
}

static void button_event_handler(nrf_drv_gpiote_pin_t pin,
                                 nrf_gpiote_polarity_t action) {
  if (pin == BUTTON) {
    // Перезапускаем таймер антидребезга (если контакт дребезжит, таймер просто
    // сдвинется)
    app_timer_stop(m_debounce_timer);
    app_timer_start(m_debounce_timer, APP_TIMER_TICKS(DEBOUNCE_DELAY_MS), NULL);
  }
}

void btn_init(void) {
  ret_code_t err_code;

  // Создаём оба таймера в режиме SINGLE_SHOT (одноразовые)
  err_code = app_timer_create(&m_debounce_timer, APP_TIMER_MODE_SINGLE_SHOT,
                              debounce_timeout_handler);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_double_click_timer, APP_TIMER_MODE_SINGLE_SHOT,
                              double_click_timeout_handler);
  APP_ERROR_CHECK(err_code);


  // TOGGLE означает, что прерывание сработает и при нажатии, и при отпускании
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP; // Внутренняя подтяжка к питанию

  err_code = nrf_drv_gpiote_in_init(BUTTON, &in_config, button_event_handler);
  APP_ERROR_CHECK(err_code);

  // Включаем отслеживание событий для пина кнопки
  nrf_drv_gpiote_in_event_enable(BUTTON, true);
}
