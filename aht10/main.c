/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

uint8_t AHT10_ADDRESS = 0x38;     // 0111000 (7bit address)
uint8_t AHT10_READ_DELAY_MS = 75; // Time it takes for AHT to collect data
uint8_t AHT_TEMPERATURE_CONST = 200;
uint8_t AHT_TEMPERATURE_OFFSET = 50;
uint16_t KILOBYTE_CONST = 1048576;
uint8_t FARENHEIT_MULTIPLIER = 9 / 5;
uint8_t FARENHEIT_OFFSET = 32;

void aht10_init()
{
    sleep_ms(100);
    uint8_t addr = 0x38;
    uint8_t data[] = {0xE1, 0x08, 0x00};
    i2c_write_blocking(i2c0, addr, data, 3, true);
    sleep_ms(100);
}

void aht10_reset()
{
    sleep_ms(100);
    uint8_t addr = 0x38;
    uint8_t data = 0xBA;
    i2c_write_blocking(i2c0, addr, &data, 1, true);
    sleep_ms(100);
}
void aht10_read_raw()
{
    sleep_ms(100);
    uint8_t addr = 0x38;
    uint8_t reg[3] = {0xAC, 0x33, 0x00};
    uint8_t val[6];
    i2c_write_blocking(i2c0, addr, reg, 3, true);
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
    printf("%d\n", val[0]);
    printf("%d\n", val[1]);
    printf("%d\n", val[2]);
    printf("%d\n", val[3]);
    printf("%d\n", val[4]);
    printf("%d\n", val[5]);
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
    aht10_reset();
    aht10_init();

    while (true)
    {
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