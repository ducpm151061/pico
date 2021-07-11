/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include <math.h>
#include "tcs3472.h"

float powf(const float x, const float y)
{
    return (float)(pow((double)x, (double)y));
}

void write(uint8_t reg, uint8_t value)
{
    sleep_ms(10);
    uint8_t data[] = {TCS34725_COMMAND_BIT | reg, value & 0xFF};
    i2c_write_blocking(i2c0, (uint8_t)TCS34725_ADDRESS, data, 2, false);
    sleep_ms(10);
}
uint8_t read(uint8_t reg)
{
    sleep_ms(10);
    uint8_t data[1];
    reg = reg | TCS34725_COMMAND_BIT;
    i2c_write_blocking(i2c0, (uint8_t)TCS34725_ADDRESS, &reg, 1, true);
    sleep_ms(10);
    i2c_read_blocking(i2c0, (uint8_t)TCS34725_ADDRESS, data, 1, false);
    sleep_ms(10);
    return data[0];
}
uint16_t reads(uint8_t reg)
{
    sleep_ms(10);
    uint16_t data = 0;
    reg = reg | TCS34725_COMMAND_BIT;
    uint8_t temp[2];
    i2c_write_blocking(i2c0, (uint8_t)TCS34725_ADDRESS, &reg, 1, true);
    sleep_ms(10);
    i2c_read_blocking(i2c0, (uint8_t)TCS34725_ADDRESS, temp, 2, false);
    data = (temp[0] << 8) | temp[1];
    sleep_ms(10);
    return data;
}

void enable()
{
    write(TCS34725_ENABLE, TCS34725_ENABLE_PON);
    write(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    sleep_ms(100);
}
void disable()
{
    uint8_t reg = read(TCS34725_ENABLE);
    write(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
}
bool checkTCS()
{
    uint8_t data = read(TCS34725_ID);
    printf("ID Register: 0x%02x\n", data);
    if (data == 0x4d)
        return true;
    else
        return false;
}
void setIntegrationTime(uint8_t it)
{
    write(TCS34725_ATIME, it);
}
void setGain(tcs34725Gain_t gain)
{
    write(TCS34725_CONTROL, gain);
}
void getRawData(uint16_t *r, uint16_t *g, uint16_t *b,
                uint16_t *c)
{
    *c = reads(TCS34725_CDATAL);
    sleep_ms(10);

    *r = reads(TCS34725_RDATAL);
    sleep_ms(10);

    *g = reads(TCS34725_GDATAL);
    sleep_ms(10);

    *b = reads(TCS34725_BDATAL);
    sleep_ms(10);
}
void getRGB(float *r, float *g, float *b)
{
    uint16_t red, green, blue, clear;
    getRawData(&red, &green, &blue, &clear);
    uint32_t sum = clear;

    // Avoid divide by zero errors ... if clear = 0 return black
    if (clear == 0)
    {
        *r = *g = *b = 0;
        return;
    }

    *r = (float)red / sum * 255.0;
    *g = (float)green / sum * 255.0;
    *b = (float)blue / sum * 255.0;
}
uint16_t calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
{
    float X, Y, Z; /* RGB to XYZ correlation      */
    float xc, yc;  /* Chromaticity co-ordinates   */
    float n;       /* McCamy's formula            */
    float cct;

    if (r == 0 && g == 0 && b == 0)
    {
        return 0;
    }

    /* 1. Map RGB values to their XYZ counterparts.    */
    /* Based on 6500K fluorescent, 3000K fluorescent   */
    /* and 60W incandescent values for a wide range.   */
    /* Note: Y = Illuminance or lux                    */
    X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
    Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
    Z = (-0.68202F * r) + (0.77073F * g) + (0.56332F * b);

    /* 2. Calculate the chromaticity co-ordinates      */
    xc = (X) / (X + Y + Z);
    yc = (Y) / (X + Y + Z);

    /* 3. Use McCamy's formula to determine the CCT    */
    n = (xc - 0.3320F) / (0.1858F - yc);

    /* Calculate the final CCT */
    cct =
        (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

    /* Return the results in degrees Kelvin */
    return (uint16_t)cct;
}

uint16_t calculateLux(uint16_t r, uint16_t g, uint16_t b)
{
    float illuminance;

    /* This only uses RGB ... how can we integrate clear or calculate lux */
    /* based exclusively on clear since this might be more reliable?      */
    illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

    return (uint16_t)illuminance;
}
void setInterrupt(bool i)
{
    uint8_t r = read(TCS34725_ENABLE);
    if (i)
    {
        r |= TCS34725_ENABLE_AIEN;
    }
    else
    {
        r &= ~TCS34725_ENABLE_AIEN;
    }
    write(TCS34725_ENABLE, r);
}
void clearInterrupt()
{
    write(TCS34725_ADDRESS, TCS34725_COMMAND_BIT | 0x66);
}

void setIntLimits(uint16_t low, uint16_t high)
{
    write(0x04, low & 0xFF);
    write(0x05, low >> 8);
    write(0x06, high & 0xFF);
    write(0x07, high >> 8);
}

void initTCS()
{
    if (checkTCS)
    {
        printf("init\r\n");
        setIntegrationTime(TCS34725_INTEGRATIONTIME_50MS);
        setGain(TCS34725_GAIN_4X);
        enable();
    }
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

    // Set up tcs
    initTCS();

    while (true)
    {
        uint16_t r, g, b, c, colorTemp, lux;

        getRawData(&r, &g, &b, &c);
        colorTemp = calculateColorTemperature(r, g, b);
        lux = calculateLux(r, g, b);
        // printf("LED ON\r\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        printf("R %u | G %u | B %u\n", r, g, b);
        printf("Lux: %u\n", lux);
        printf("Color Temp: %u\n", colorTemp);
        // printf("LED OFF\r\n");
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
#endif
}