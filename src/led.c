#include "led.h"
#include "nrf_gpio.h"
#include <stdint.h>

#define LED1_Y         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define LEDS_LIST { LED1_Y, LED2_R, LED2_G, LED2_B }

static const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

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

  for (uint32_t i = 0; i < LEDS_NUMBER; ++i) {
    nrf_gpio_cfg_output(leds_list[i]);
    led_idx_off(i);
  }
}

void led_idx_on(uint32_t led_idx) {
  ASSERT(led_idx < LEDS_NUMBER);
  nrf_gpio_pin_write(leds_list[led_idx], LEDS_ACTIVE_STATE ? 1 : 0);
}

void led_idx_off(uint32_t led_idx) {
  ASSERT(led_idx < LEDS_NUMBER);
  nrf_gpio_pin_write(leds_list[led_idx], LEDS_ACTIVE_STATE ? 0 : 1);
}
