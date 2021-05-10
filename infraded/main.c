#include "lcd_1602_i2c.h"
#include "pico/stdlib.h"

int main()
{

    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    lcd_init();

    while (1)
    {
        char ftemp[15];
        // sprintf(ftemp, "Temp = %.1f F", fahrenheit);
        char humid[15];
        // sprintf(humid, "Humidity = %.1f%%", reading.humidity);
        //show temp and humidity on display
        lcd_set_cursor(0,0);  //line 1 of the LCD
        lcd_string(ftemp);
        lcd_set_cursor(1,0); // line 2 of the LCD
        lcd_string(humid);

        sleep_ms(2000); // pause for 2 seconds, prevents LCD from refreshing too often
        lcd_clear();
    }
    return 0;
}