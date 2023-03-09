/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ds1307.h"

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  // Set up I2C

  while (true) {
    printf("LED ON\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    printf("LED OFF\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
  }
#endif
}