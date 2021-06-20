/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/adc.h"

// Core 1 interrupt Handler
void core1_interrupt_handler()
{
    // Receive Raw Value, Convert and Print Temperature Value
    while (multicore_fifo_rvalid())
    {
        uint16_t raw = multicore_fifo_pop_blocking();
        const float conversion_factor = 3.3f / (1 << 12);
        float result = raw * conversion_factor;
        float temp = 27 - (result - 0.706) / 0.001721;
        printf("Temp = %f C\n", temp);
    }
    multicore_fifo_clear_irq(); // Clear interrupt
}

// Core 1 Main Code
void core1_entry()
{
    // Configure Core 1 Interrupt
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    // Infinite while loop to wait for interrupt
    while (true)
    {
        tight_loop_contents();
    }
}

int main()
{
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    stdio_init_all();
    multicore_launch_core1(core1_entry); // Stare core 1- Do this before any interrupt
    // Configure the ADC
    adc_init();
    adc_set_temp_sensor_enabled(true); // Enable on board temp sensor
    adc_select_input(4);
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true)
    {
        printf("LED ON\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        uint16_t raw = adc_read();
        multicore_fifo_push_blocking(raw);
        printf("LED OFF\n");
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
#endif
}