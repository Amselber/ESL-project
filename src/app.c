#include "app.h"
#include "btn.h"
#include "led.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include <stdbool.h>
#include <stdint.h>

#define FADE_STEP 5     // Шаг изменения яркости (скорость)
#define STEP_DELAY_MS 2 // Длительность одного тика автомата (2 мс)
#define HOLD_TICKS 50   // Сколько тиков держать 100% или 0% (50 * 2 мс = 100 мс)

// device ID: #6593
#define LEDS_BLINKS {6, 5, 9, 3}

typedef enum {
  STATE_FADE_IN,  // Плавное разгорание (0% -> 100%)
  STATE_LED_ON,   // Светодиод полностью горит (100%)
  STATE_FADE_OUT, // Плавное затухание (100% -> 0%)
  STATE_LED_OFF   // Светодиод полностью выключен (0%)
} blink_state_t;

static const uint8_t default_counter[LEDS_NUMBER] = LEDS_BLINKS;
static uint8_t curr_counter[LEDS_NUMBER] = LEDS_BLINKS;
static uint8_t curr_idx = 0;

// Переменные состояния (сохраняются при остановке)
static blink_state_t state      = STATE_FADE_IN;
static uint16_t      curr_duty  = 0;
static uint16_t      hold_ticks = 0;

// Локальный флаг активности (управляется только через callback кнопки)
static volatile bool is_blinking_active = false;

/**
 * @brief Callback-обработчик событий от модуля btn.c
 */
static void on_button_event(btn_event_t event) {
  if (event == BTN_EVENT_DOUBLE_CLICK) {
    is_blinking_active = !is_blinking_active;
  }
}

/**
 * @brief Application init
 * run before app_process()
 */
void app_init(void) {
  led_init();
  btn_init(on_button_event); // Регистрируем callback в модуле кнопки
  NRF_LOG_INFO("[APP] Initialized");
}

/**
 * @brief Application process
 * run in main loop
 */
void app_process(void) {
  // Если режим мигания выключен — просто выходим.
  // Все переменные (state, curr_duty, curr_idx, hold_ticks) сохраняют свои значения!
  if (!is_blinking_active) {
    return;
  }

  // Проверяем, не закончилась ли последовательность всех 4 светодиодов
  if (curr_idx >= LEDS_NUMBER) {
    curr_idx = 0;
    return;
  }

  // Если текущий светодиод отмигал своё количество раз — переходим к следующему
  if (curr_counter[curr_idx] == 0) {
    curr_counter[curr_idx] = default_counter[curr_idx];
    ++curr_idx;
    state = STATE_FADE_IN;
    return;
  }

  // Основной конечный автомат
  switch (state) {
  case STATE_FADE_IN:
    curr_duty += FADE_STEP;
    if (curr_duty >= PWM_MAX_DUTY) {
      curr_duty = PWM_MAX_DUTY;
      led_idx_on(curr_idx);    // Явное включение на 100%
      hold_ticks = HOLD_TICKS; // Заряжаем таймер удержания
      state = STATE_LED_ON;
    } else {
      led_idx_set_duty(curr_idx, curr_duty);
    }
    break;

  case STATE_LED_ON:
    if (hold_ticks > 0) {
      --hold_ticks;
    } else {
      state = STATE_FADE_OUT;
    }
    break;

  case STATE_FADE_OUT:
    if (curr_duty > FADE_STEP) {
      curr_duty -= FADE_STEP;
      led_idx_set_duty(curr_idx, curr_duty);
    } else {
      curr_duty = 0;
      led_idx_off(curr_idx); // Явное выключение (0%)
      hold_ticks = HOLD_TICKS;
      state = STATE_LED_OFF;
    }
    break;

  case STATE_LED_OFF:
    if (hold_ticks > 0) {
      --hold_ticks;
    } else {
      --curr_counter[curr_idx]; // Один полный цикл мигания завершён
      state = STATE_FADE_IN;
    }
    break;
  }

  nrf_delay_ms(STEP_DELAY_MS);
}
