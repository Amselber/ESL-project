#include "led.h"
#include "nrf_gpio.h"
#include "nrfx_pwm.h"
#include <stdint.h>

#define LED1_Y NRF_GPIO_PIN_MAP(0, 6)
#define LED2_R NRF_GPIO_PIN_MAP(0, 8)
#define LED2_G NRF_GPIO_PIN_MAP(1, 9)
#define LED2_B NRF_GPIO_PIN_MAP(0, 12)

static nrfx_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_individual_t m_pwm_values;

static nrf_pwm_sequence_t const m_pwm_seq = {
    .values.p_individual = &m_pwm_values,
    .length = NRF_PWM_VALUES_LENGTH(m_pwm_values),
    .repeats = 0,
    .end_delay = 0};

static void gpio_output_voltage_setup(void) {
  // Configure UICR_REGOUT0 register only if it is set to default value.
  if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
      (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos)) {
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
    }

    NRF_UICR->REGOUT0 =
        (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
        (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
    }

    // System reset is needed to update UICR registers.
    NVIC_SystemReset();
  }
}

void led_init(void) {
  // If nRF52 USB Dongle is powered from USB (high voltage mode),
  // GPIO output voltage is set to 1.8 V by default, which is not
  // enough to turn on green and blue LEDs. Therefore, GPIO voltage
  // needs to be increased to 3.0 V by configuring the UICR register.
  if (NRF_POWER->MAINREGSTATUS & (POWER_MAINREGSTATUS_MAINREGSTATUS_High
                                  << POWER_MAINREGSTATUS_MAINREGSTATUS_Pos)) {
    gpio_output_voltage_setup();
  }

  uint32_t led_pins[LEDS_NUMBER] = {
      LED1_Y | NRFX_PWM_PIN_INVERTED, LED2_R | NRFX_PWM_PIN_INVERTED,
      LED2_G | NRFX_PWM_PIN_INVERTED, LED2_B | NRFX_PWM_PIN_INVERTED};

  nrfx_pwm_config_t config = {
      .output_pins = {led_pins[0], led_pins[1], led_pins[2], led_pins[3]},
      .irq_priority = APP_IRQ_PRIORITY_LOWEST,
      .base_clock = NRF_PWM_CLK_1MHz, // 1 МГц / 1000 = 1 кГц ШИМ
      .count_mode = NRF_PWM_MODE_UP,
      .top_value = PWM_MAX_DUTY,
      .load_mode = NRF_PWM_LOAD_INDIVIDUAL,
      .step_mode = NRF_PWM_STEP_AUTO};

  APP_ERROR_CHECK(nrfx_pwm_init(&m_pwm0, &config, NULL));

  // Выключаем все светодиоды на старте
  for (uint8_t i = 0; i < LEDS_NUMBER; i++) {
    led_idx_off(i);
  }

  // Запускаем аппаратный ШИМ в бесконечном цикле
  nrfx_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void led_idx_set_duty(uint32_t led_idx, uint16_t duty) {
    if (led_idx >= LEDS_NUMBER) return;
    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;

    uint16_t *channels = (uint16_t *)&m_pwm_values;
    channels[led_idx] = duty;
}

void led_idx_on(uint32_t led_idx) {
  led_idx_set_duty(led_idx, PWM_MAX_DUTY); // 100% яркости
}

void led_idx_off(uint32_t led_idx) {
  led_idx_set_duty(led_idx, 0); // 0% яркости
}
