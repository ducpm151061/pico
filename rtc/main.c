/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000
#define DS1307_I2C_ADD 0x68

void aht10_init() {
  sleep_ms(100);
  uint8_t addr = 0x38;
  uint8_t data[] = {0xE1, 0x08, 0x00};
  i2c_write_blocking(i2c0, DS1307_I2C_ADD, data, 3, false);
  sleep_ms(100);
}

void aht10_reset() {
  sleep_ms(100);
  uint8_t addr = 0x38;
  uint8_t data = 0xBA;
  i2c_write_blocking(i2c0, addr, &data, 1, false);
  sleep_ms(100);
}
void aht10_read_raw() {
  sleep_ms(100);
  uint8_t addr = 0x38;
  uint8_t reg[3] = {0xAC, 0x33, 0};
  uint8_t val[6];
  i2c_write_blocking(i2c0, addr, reg, 3, false);
  sleep_ms(100);
  i2c_read_blocking(i2c0, addr, val, 6, false);
  sleep_ms(100);
  uint32_t h = val[1];
  h <<= 8;
  h |= val[2];
  h <<= 4;
  h |= val[3] >> 4;
  float humidity = ((float)h * 100) / 0x100000;

  uint32_t tdata = val[3] & 0x0F;
  tdata <<= 8;
  tdata |= val[4];
  tdata <<= 8;
  tdata |= val[5];

  float temperature = ((float)tdata * 200 / 0x100000) - 50;
  printf("Temperature: %f\n", temperature);
  printf("Humidity: %f\n", humidity);
}

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  // Set up I2C
  i2c_init(I2C_PORT, I2C_FREQUENCY);
  gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
  gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_GPIO);
  gpio_pull_up(SCL_GPIO);
  aht10_reset();
  aht10_init();

  while (true) {
    printf("LED ON\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    aht10_read_raw();
    printf("LED OFF\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
  }
#endif
}