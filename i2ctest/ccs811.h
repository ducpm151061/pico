
#include "hardware/i2c.h"
#include <stdio.h>

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
#define I2C_PORT i2c1

void ccs811_check_id(void);
void ccs811_check_version(void);
void ccs811_read_data(void);
void ccs811_read_raw_data(void);
