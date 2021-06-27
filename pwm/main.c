/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include <stdio.h>
#ifdef PICO_DEFAULT_LED_PIN
void on_pwm_wrap()
{
    static int fade = 0;
    static bool going_up = true;

    pwm_clear_irq(pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN));

    if (going_up)
    {
        ++fade;
        if (fade > 255)
        {
            fade = 255;
            going_up = false;
        }
    }
    else
    {
        --fade;
        if (fade < 0)
        {
            fade = 0;
            going_up = true;
        }
    }
    pwm_set_gpio_level(PICO_DEFAULT_LED_PIN,fade*fade);
}
#endif

int main()
{
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    stdio_init_all();
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_set_function(LED_PIN,GPIO_FUNC_PWM);
    uint slice_num=pwm_gpio_to_slice_num(LED_PIN);
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num,true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP,on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP,true);


    pwm_config config=pwm_get_default_config();
    pwm_config_set_clkdiv(&config,4.f);
    pwm_init(slice_num,&config,true);
    while (true)
    {
        printf("LED ON");
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        tight_loop_contents();
        printf("LED OFF");
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
#endif
}