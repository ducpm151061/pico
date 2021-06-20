#include "pico/stdlib.h"
#include <stdio.h>
#include "ccs811.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
int main()
{
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // init i2c
    i2c_init(I2C_PORT, 10 * 1000);
    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);
    gpio_pull_up(6);
    gpio_pull_up(7);
    // Make the I2C pins available to picotool

    // init ccs811 sensor
    ccs811_check_id();
    ccs811_check_version();

    while (true)
    {
        // printf("LED ON\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        // printf("LED OFF\n");
        ccs811_read_data();
        ccs811_read_raw_data();
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}