/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bh1750.h"

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  BH1750 bh1750;
  bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  while (true) {
    printf("-----------------------\n");
    printf("LED ON\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(250);

    if (bh1750.measurementReady()) {
      float lux = bh1750.readLightLevel();
      printf("Light: %f lux", lux);
    }

    printf("LED OFF\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    printf("-----------------------\n");
  }
#endif
}