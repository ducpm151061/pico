#include "pico/stdlib.h"
#include <stdio.h>
#include "lora.h"
#include "bmp180.h"

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
    setup_lora(434, 2, "CODEBRANE");
    unsigned char loraBuffer[25] = "Hello From Pico!";

#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);

        if (!bmp_get_pressure_temperature(&bmp))
            return 1;

        printf("BMP180 Temperature (C): %f\n", bmp.temperature);
        sprintf(loraBuffer, "Nhiet do la : %f", bmp.temperature);
        SendLoRaPacket(loraBuffer, 25, 0);

        sleep_ms(500);


        printf("BMP180 Pressure (hPa): %f\n", (float)bmp.pressure / 100.0);
        sprintf(loraBuffer, "Ap suat la: %f", (float)bmp.pressure / 100.0);
        SendLoRaPacket(loraBuffer, 25, 0);

        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}
#endif