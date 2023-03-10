/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "at45db.h"

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();

  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  At45db at45db;
  at45db.init();
  uint8_t result[4];
  uint8_t data[] = {'a', 'n', 'h', 'l', 'a', 'd', 'u', 'c', '\0'};
  at45db.writeStrBuf1(0x13, data, sizeof(data));

  while (true) {
    printf("LED ON\r\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(500);

    at45db.readID(result);
    printf("result: %#x %#x %#x %#x\r\n", result[0], result[1], result[2],
           result[3]);
    uint8_t data[9];
    at45db.readStrBuf1(0x13, data, sizeof(data));

    printf("data: %s\r\n", data);
    printf("LED OFF\r\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(500);
  }
#endif
}