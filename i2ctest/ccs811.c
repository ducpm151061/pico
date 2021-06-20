#include "ccs811.h"

void ccs811_check_id(void)
{
    sleep_ms(1000);
    uint8_t reg = CSS811_HW_ID;
    uint8_t chipID[1];
    i2c_write_blocking(I2C_PORT, CCS811_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, CCS811_ADDR, chipID, 1, false);
    if (chipID[0] != 0x81)
    {
        printf("Chip Id not correct!");
    }
    else
        printf("OK, continue\n");
}

void ccs811_check_version(void)
{
    sleep_ms(1000);
    uint8_t reg = CSS811_HW_VERSION;
    uint8_t chipVersion[1];
    i2c_write_blocking(I2C_PORT, CCS811_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, CCS811_ADDR, chipVersion, 1, false);
    if (chipVersion[0] < 0x10 || chipVersion[0] > 0x1F)
    {
        printf("Chip Version not correct!");
    }
    else
        printf("OK, continue\n");
}

void ccs811_read_data(void){
     sleep_ms(1000);
    uint8_t reg = CSS811_ALG_RESULT_DATA;
    uint8_t val[4];
    i2c_write_blocking(I2C_PORT, CCS811_ADDR, &reg, 4, true);
    i2c_read_blocking(I2C_PORT, CCS811_ADDR, val, 4, false);
    printf("data ");
    printf(" %d ",val[0]);
    printf("%d  ",val[1]);
    printf("%d  ",val[2]);
    printf("%d  \n",val[3]);
}


void ccs811_read_raw_data(void){
    sleep_ms(1000);
    uint8_t reg = CSS811_RAW_DATA;
    uint8_t val[2];
    i2c_write_blocking(I2C_PORT, CCS811_ADDR, &reg, 2, true);
    i2c_read_blocking(I2C_PORT, CCS811_ADDR, val, 2, false);
    printf("raw data ");
    printf(" %d  ",val[0]);
    printf("%d\n",val[1]);
}