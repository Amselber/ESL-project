/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be
 * reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include "gpio.h"
#include "nrf_delay.h"
#include "pca10059.h"
#include "src/gpio.h"
#include <stdint.h>

#define BLINK_MS 300  // on-off led duration
#define PAUSE_MS 500  // pause between dif leds
#define CYCLE_MS 500 // pause between new cycle

#define LEDS_BLINKS {6, 5, 9, 3};

/**
 * @brief Blink a single LED a given number of times.
 *
 * @param[in] led_idx Order number for led
 * @param[in] count Number of on/off blink cycles to perform.
 */
static void blink_led(uint32_t led_idx) {
  led_idx_on(led_idx);
  nrf_delay_ms(BLINK_MS);
  led_idx_off(led_idx);
}

/**
 * @brief Function for application main entry.
 */
int main(void) {
  /* Configure board. */
  leds_init();
  btn_init();

  // device ID: #6593
  const int leds_curr_counter_default[LEDS_NUMBER] = LEDS_BLINKS;
  int leds_curr_counter[LEDS_NUMBER] = LEDS_BLINKS;
  int leds_curr_idx = 0;

  // Start delay
  nrf_delay_ms(CYCLE_MS + 500);
  while (true) {
    if (btn_get_state()) {
        // led_idx_off(leds_curr_idx);
        if (leds_curr_idx < LEDS_NUMBER) {
            if (leds_curr_counter[leds_curr_idx] > 0) {
                blink_led(leds_curr_idx);
                --leds_curr_counter[leds_curr_idx];
            } else {
                leds_curr_counter[leds_curr_idx] = leds_curr_counter_default[leds_curr_idx];
                ++leds_curr_idx;
            }
        } else {
            leds_curr_idx = 0;
        }
    } else {
        // led_idx_on(leds_curr_idx);
    }

    nrf_delay_ms(CYCLE_MS);
  }
}

/**
 *@}
 **/
