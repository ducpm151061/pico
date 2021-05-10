#include <stdio.h>
#include "bmp180.h"
#include "pico/stdlib.h"

int main()
{
    // INIT I/O
    stdio_init_all();
    bmp_t bmp;
    bmp.oss = 0;
    bmp.i2c.addr = 0x77;
    bmp.i2c.inst = i2c1;
    bmp.i2c.rate = 400000;
    bmp.i2c.scl = 27;
    bmp.i2c.sda = 26;

    if (!bmp_init(&bmp))
        return 1;

#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true)
    {
        gpio_put(LED_PIN, 1);
        printf("LED ON!\n");
        sleep_ms(250);


        if (!bmp_get_pressure_temperature(&bmp))
            return 1;
        printf("BMP180 Temperature (C): %f\n", bmp.temperature);
        printf("BMP180 Pressure (hPa): %f\n", (float)bmp.pressure / 100.0);
        
        gpio_put(LED_PIN, 0);
        printf("LED OFF!\n");
        printf("------------------------------\n");
        sleep_ms(250);

    }
#endif
}