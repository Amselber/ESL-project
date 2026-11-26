#include "app.h"
#include "btn.h"
#include "led.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include <stdbool.h>
#include <stdint.h>

// Индексы каналов ШИМ из led.c
#define LED1_MODE_IDX 0 // LED1 (жёлтый) — индикатор режима
#define LED2_R_IDX 1    // LED2 Red
#define LED2_G_IDX 2    // LED2 Green
#define LED2_B_IDX 3    // LED2 Blue

// Device ID #6593 -> 93% -> Hue = 360 * 0.93 = 334.8° -> 335°
#define DEFAULT_HUE 335
#define DEFAULT_SAT 100
#define DEFAULT_VAL 100

// Тайминги автомата (шаг 20 мс)
#define LOOP_DELAY_MS 20
#define BLINK_SLOW_TICKS 25 // 25 * 20 мс = 500 мс (медленное мигание — Hue)
#define BLINK_FAST_TICKS 6  // 6 * 20 мс = 120 мс (быстрое мигание — Saturation)

typedef enum {
  MODE_NO_INPUT = 0, // LED1 всегда выключен
  MODE_HUE,          // LED1 мигает медленно
  MODE_SATURATION,   // LED1 мигает быстро
  MODE_BRIGHTNESS,   // LED1 всегда горит
  MODE_COUNT
} input_mode_t;

static const char *mode_names[MODE_COUNT] = {"No input", "Hue modification",
                                             "Saturation modification",
                                             "Brightness modification"};

// Текущие параметры HSV
static uint16_t curr_h = DEFAULT_HUE; // 0..359
static uint8_t curr_s = DEFAULT_SAT;  // 0..100
static uint8_t curr_v = DEFAULT_VAL;  // 0..100

// Направление изменения для S и V (-1 вниз, +1 вверх)
static int8_t sat_dir = -1;
static int8_t val_dir = -1;

// Флаги состояния (изменяются в callback-обработчике кнопки)
static volatile input_mode_t curr_mode = MODE_NO_INPUT;
static volatile bool is_holding = false;
static volatile bool mode_changed_flag = false;
static volatile bool hold_stopped_flag = false;

// Переменные мигания LED1
static uint16_t blink_ticks = 0;
static bool led1_state = false;

/**
 * @brief Callback-обработчик событий от модуля btn.c
 */
static void on_button_event(btn_event_t event) {
  switch (event) {
  case BTN_EVENT_DOUBLE_CLICK:
    curr_mode = (input_mode_t)((curr_mode + 1) % MODE_COUNT);
    mode_changed_flag = true;
    break;

  case BTN_EVENT_HOLD_START:
    is_holding = true;
    break;

  case BTN_EVENT_HOLD_STOP:
    is_holding = false;
    hold_stopped_flag = true;
    break;
  }
}

/**
 * @brief Целочисленный перевод HSV (H: 0..359, S: 0..100, V: 0..100)
 *        в скважность ШИМ (0..PWM_MAX_DUTY) и вывод на LED2
 */
static void update_led2_hsv(uint16_t h, uint8_t s, uint8_t v) {
  h %= 360;

  if (s == 0) {
    uint16_t gray = ((uint32_t)v * PWM_MAX_DUTY) / 100;
    led_idx_set_duty(LED2_R_IDX, gray);
    led_idx_set_duty(LED2_G_IDX, gray);
    led_idx_set_duty(LED2_B_IDX, gray);
    return;
  }

  uint8_t region = h / 60;
  uint8_t remainder = h % 60;

  uint32_t v_scaled = ((uint32_t)v * PWM_MAX_DUTY) / 100;
  uint16_t p = (v_scaled * (100 - s)) / 100;
  uint16_t q = (v_scaled * (6000 - (uint32_t)s * remainder)) / 6000;
  uint16_t t = (v_scaled * (6000 - (uint32_t)s * (60 - remainder))) / 6000;

  uint16_t r = 0, g = 0, b = 0;
  switch (region) {
  case 0:
    r = v_scaled;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v_scaled;
    b = p;
    break;
  case 2:
    r = p;
    g = v_scaled;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v_scaled;
    break;
  case 4:
    r = t;
    g = p;
    b = v_scaled;
    break;
  default:
    r = v_scaled;
    g = p;
    b = q;
    break;
  }

  led_idx_set_duty(LED2_R_IDX, r);
  led_idx_set_duty(LED2_G_IDX, g);
  led_idx_set_duty(LED2_B_IDX, b);
}

/**
 * @brief Обновление индикации текущего режима на LED1
 */
static void update_led1_indication(void) {
  switch (curr_mode) {
  case MODE_NO_INPUT:
    led_idx_off(LED1_MODE_IDX);
    break;

  case MODE_HUE:
    if (++blink_ticks >= BLINK_SLOW_TICKS) {
      blink_ticks = 0;
      led1_state = !led1_state;
      led1_state ? led_idx_on(LED1_MODE_IDX) : led_idx_off(LED1_MODE_IDX);
    }
    break;

  case MODE_SATURATION:
    if (++blink_ticks >= BLINK_FAST_TICKS) {
      blink_ticks = 0;
      led1_state = !led1_state;
      led1_state ? led_idx_on(LED1_MODE_IDX) : led_idx_off(LED1_MODE_IDX);
    }
    break;

  case MODE_BRIGHTNESS:
    led_idx_on(LED1_MODE_IDX);
    break;

  default:
    break;
  }
}

void app_init(void) {
  led_init();
  btn_init(on_button_event);

  // Зажигаем стартовый цвет HSV(335, 100, 100) при подаче питания
  update_led2_hsv(curr_h, curr_s, curr_v);
  led_idx_off(LED1_MODE_IDX);

  NRF_LOG_INFO("[APP] Initialized. Default HSV(%d, %d, %d)", curr_h, curr_s,
               curr_v);
}

void app_process(void) {
  // Обработка смены режима по двойному клику
  if (mode_changed_flag) {
    mode_changed_flag = false;
    blink_ticks = 0;
    led1_state = true;

    if (curr_mode == MODE_NO_INPUT) {
      led_idx_off(LED1_MODE_IDX);
    } else {
      led_idx_on(LED1_MODE_IDX);
    }

    NRF_LOG_INFO("[APP] Mode -> %s", mode_names[curr_mode]);
  }

  // Плавное изменение параметра при удержании кнопки
  if (is_holding && curr_mode != MODE_NO_INPUT) {
    switch (curr_mode) {
    case MODE_HUE:
      curr_h = (curr_h + 1) % 360;
      break;

    case MODE_SATURATION:
      if (curr_s == 100)
        sat_dir = -1;
      else if (curr_s == 0)
        sat_dir = 1;
      curr_s = (uint8_t)((int8_t)curr_s + sat_dir);
      break;

    case MODE_BRIGHTNESS:
      if (curr_v == 100)
        val_dir = -1;
      else if (curr_v == 0)
        val_dir = 1;
      curr_v = (uint8_t)((int8_t)curr_v + val_dir);
      break;

    default:
      break;
    }

    update_led2_hsv(curr_h, curr_s, curr_v);
  }

  // 3. Фиксация итогового цвета в лог при отпускании кнопки
  if (hold_stopped_flag) {
    hold_stopped_flag = false;
    if (curr_mode != MODE_NO_INPUT) {
      NRF_LOG_INFO("[APP] New HSV(%d, %d, %d)", curr_h, curr_s, curr_v);
    }
  }

  // Индикация текущего режима на LED1
  update_led1_indication();

  nrf_delay_ms(LOOP_DELAY_MS);
}
