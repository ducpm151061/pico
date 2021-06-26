/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "KalmanFilter.h"

float average_adc(uint16_t times)
{
    uint32_t temp = 0;
    uint16_t i;
    for (i = 0; i < times; i++)
    {
        temp += adc_read();
    }
    return temp / times;
}

int main()
{
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    stdio_init_all();
    // init adc
    adc_init();
    adc_gpio_init(28);
    adc_select_input(0);

    // init kalman filter
    float e_mea = 1;
    float e_est = 1;
    float q = 0.01;
    KalmanFilter kf = KalmanFilter(e_mea, e_est, q);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true)
    {
        const float conversion_factor = 5.0f / (1 << 10);
        float raw = average_adc(100);
        float result = (float)(kf.updateEstimate(raw));
        printf("LED ON\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        printf("Raw value: %f, voltage: %f V\n", result, result * conversion_factor);
        printf("LED OFF\n");
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
#endif
}