#include "ds1307.h"
#include <stdio.h>
#ifndef PICO_DEFAULT_LED_PIN
#error blink example requires a board with a regular LED
#endif

#define SDA_PIN 2
#define SCL_PIN 3

int main() {
  stdio_init_all();
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  DS1307 ds1307;
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_PIN);
  gpio_pull_up(SCL_PIN);
  ds1307.begin(i2c1);
  // ds1307.DSadjust(0, 15, 21, 2022, 3, 16); // 00:15:21 16 Mar 2022

  while (true) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(250);
    ds1307.DSread();
    char *time = new char[256];
    ds1307.DSgetTime(time);
    printf("Time: %s\r\n", time);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(250);
  }
}
