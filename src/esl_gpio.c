#include "esl_gpio.h"
#include "nrf_gpio.h"
#include "pca10059.h"
#include <stdint.h>

#if LEDS_NUMBER > 0
static const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;
#endif

#if BUTTONS_NUMBER > 0
static const uint8_t btns_list[BUTTONS_NUMBER] = BUTTONS_LIST;
#endif

static void gpio_output_voltage_setup(void)
{
    // Configure UICR_REGOUT0 register only if it is set to default value.
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
        (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
    }
}

void leds_init(void) {
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

bool btn_get_state(uint32_t btn_idx) {
  bool pin_set = nrf_gpio_pin_read(btns_list[btn_idx]) ? true : false;
  return (pin_set == (BUTTONS_ACTIVE_STATE ? true : false));
}

void btns_init(void) {
  for (uint32_t i = 0; i < BUTTONS_NUMBER; ++i) {
    nrf_gpio_cfg_input(btns_list[i], BUTTON_PULL);
  }
}
