/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"
uint8_t addr = 0x23;

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

void light_on()
{
    sleep_ms(500);
    uint8_t data[1];
    data[0] = 0x01;
    i2c_write_blocking(i2c0, addr, data, 1, true);
}
void light_low_reg()
{
    sleep_ms(100);
    uint8_t reg = 0x21;
    uint8_t val[1];
    uint16_t res[1];
    i2c_write_blocking(i2c0, addr, &reg, 1, true);
    sleep_ms(150);
    i2c_read_blocking(i2c0, addr, val, 2, false);
    printf("%d\n", val[0]);
    printf("%d\n", val[1]);
    res[0] = (val[0] << 8);
    res[0] |= val[1];
    printf("%lu\n", res[0]/1.2);
}

int main()
{
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

    while (true)
    {
        printf("-----------------------\n");
        printf("LED ON\n");
        gpio_put(LED_PIN, 1);
        light_on();
        light_low_reg();
        printf("LED OFF\n");
        gpio_put(LED_PIN, 0);
        printf("-----------------------\n");
    }
#endif
}