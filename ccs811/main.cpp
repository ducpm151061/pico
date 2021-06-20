#include <stdio.h>
#include "pico/stdlib.h"
// #include "SparkFunCCS811.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

#define CSS811_STATUS 0x00
#define CSS811_MEAS_MODE 0x01
#define CSS811_ALG_RESULT_DATA 0x02
#define CSS811_RAW_DATA 0x03
#define CSS811_ENV_DATA 0x05
#define CSS811_NTC 0x06 //NTC compensation no longer supported
#define CSS811_THRESHOLDS 0x10
#define CSS811_BASELINE 0x11
#define CSS811_HW_ID 0x20
#define CSS811_HW_VERSION 0x21
#define CSS811_FW_BOOT_VERSION 0x23
#define CSS811_FW_APP_VERSION 0x24
#define CSS811_ERROR_ID 0xE0
#define CSS811_APP_START 0xF4
#define CSS811_SW_RESET 0xFF
#define CCS811_ADDR 0x5B

void checkHWId()
{
    sleep_ms(500);
    uint8_t reg = (uint8_t)CSS811_HW_ID;
    uint8_t val[1];
    uint8_t addr = (uint8_t)CCS811_ADDR;
    i2c_write_blocking(i2c0, addr, &reg, 1, true);
    sleep_ms(150);
    i2c_read_blocking(i2c0, addr, val, 1, false);
    printf("Hardware Id: %d\n", val[0]);
}

void checkHWVersion()
{
    sleep_ms(500);
    uint8_t reg = (uint8_t)CSS811_HW_VERSION;
    uint8_t val[1];
    uint8_t addr = (uint8_t)CCS811_ADDR;
    i2c_write_blocking(i2c0, addr, &reg, 1, true);
    sleep_ms(150);
    i2c_read_blocking(i2c0, addr, val, 1, false);
    printf("Hardware Version: %d\n", val[0]);
}
void setMode()
{
    sleep_ms(500);
    uint8_t data[2];
    data[0] = (uint8_t)CSS811_MEAS_MODE;
    data[1] = 0x10;
    uint8_t addr = (uint8_t)CCS811_ADDR;
    i2c_write_blocking(i2c0, addr, data, 2, true);
}
void readMode()
{
    sleep_ms(500);
    uint8_t reg = (uint8_t)CSS811_MEAS_MODE;
    uint8_t addr = (uint8_t)CCS811_ADDR;
    uint8_t val[1];

    i2c_write_blocking(i2c0, addr, &reg, 1, true);
    i2c_read_blocking(i2c0, addr, val, 1, false);
    printf("Mode: %d\n", val[0]);
}
void readRawData()
{
    sleep_ms(500);
    uint8_t reg = (uint8_t)CSS811_ALG_RESULT_DATA;
    uint8_t val[2];
    uint8_t addr = (uint8_t)CCS811_ADDR;
    i2c_write_blocking(i2c0, addr, &reg, 1, true);
    i2c_read_blocking(i2c0, addr, val, 2, false);
    uint8_t current=val[0]>> 2;
    uint16_t voltage= uint16_t((val[0] & 0b00000011)<<8)|val[1];
    printf("Raw Data: %d %u\n", current,voltage);
}

void readAlgorithmData()
{
    sleep_ms(500);
    uint8_t reg = (uint8_t)CSS811_RAW_DATA;
    uint8_t val[8];
    uint8_t addr = (uint8_t)CCS811_ADDR;
    i2c_write_blocking(i2c0, addr, &reg, 1, true);
    i2c_read_blocking(i2c0, addr, val, 2, false);
    
    uint16_t eC02= (val[0] <<8)|val[1];
    uint16_t tvoc= (val[2] <<8)|val[3];
    uint8_t status= val[4];
    uint8_t errorId= val[5];


    printf("Algorithm Results Register: %u %u %d %d\n", eC02,tvoc,status,errorId);
}
int main()
{
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

    while (true)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        // data
        checkHWId();
        checkHWVersion();
        setMode();

        readMode();
        readRawData();
        readAlgorithmData();
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}