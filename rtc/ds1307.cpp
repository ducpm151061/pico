#include "ds1307.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
uint8_t DS1307::decToBcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }

uint8_t DS1307::bcdToDec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }

void DS1307::begin() {
  _i2c = I2C_PORT;
  i2c_init(_i2c, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
}

void DS1307::startClock(void) {
  uint8_t buff1 = 0x00;
  i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, &buff1, 1, false);
  sleep_ms(100);
  i2c_read_blocking(_i2c, DS1307_I2C_ADDRESS, &second, 1, false);
  second = second & 0x7f;
  sleep_ms(100);
  uint8_t buff2[] = {0x00, second};
  i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, buff2, 2, false);
  sleep_ms(100);
}
void DS1307::stopClock(void) {
  uint8_t buff1 = 0x00;
  i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, &buff1, 1, false);
  sleep_ms(100);
  i2c_read_blocking(_i2c, DS1307_I2C_ADDRESS, &second, 1, false);
  sleep_ms(100);
  second = second | 0x80;
  uint8_t buff2[] = {0x00, second};
  i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, buff2, 2, false);
  sleep_ms(100);
}
void DS1307::getTime() {
  uint8_t buff = 0x00;
  i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, &buff, 1, true);
  uint8_t data[7];
  i2c_read_blocking(_i2c, DS1307_I2C_ADDRESS, data, 7, false);
  second = bcdToDec(data[0] & 0x7f);
  minute = bcdToDec(data[1]);
  hour = bcdToDec(data[2] & 0x3f);
  dayOfWeek = bcdToDec(data[3]);
  dayOfMonth = bcdToDec(data[4]);
  month = bcdToDec(data[5]);
  year = bcdToDec(data[6]);
  sleep_ms(100);
}
void DS1307::setTime(time_t t) {
  uint8_t buff1[] = {0x00,
                     decToBcd(t.second),
                     decToBcd(minute),
                     decToBcd(hour),
                     decToBcd(dayOfWeek),
                     decToBcd(dayOfMonth),
                     decToBcd(month),
                     decToBcd(year)};
  i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, buff1, 8, false);
  // uint8_t buff2[] = {0x00, decToBcd(second)};
  // i2c_write_blocking(_i2c, DS1307_I2C_ADDRESS, buff2, 2, true);
  sleep_ms(100);
}
void DS1307::fillByHMS(uint8_t _hour, uint8_t _minute, uint8_t _second) {
  hour = _hour;
  minute = _minute;
  second = _second;
}
void DS1307::fillByYMD(uint16_t _year, uint8_t _month, uint8_t _day) {
  year = _year - 2000;
  month = _month;
  dayOfMonth = _day;
}
void DS1307::fillDayOfWeek(uint8_t _dow) { dayOfWeek = _dow; }
