/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  while (true) {
    printf("LED ON\r\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    printf("LED OFF\r\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
  }
#endif
}