
#include "ds1307.h"

#define DS1307_ADDRESS 0x68 ///< I2C address for DS1307
#define DS1307_CONTROL 0x07 ///< Control register
#define DS1307_NVRAM 0x08   ///< Start of RAM registers - 56 bytes, 0x08 to 0x3f

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

void RTC_DS1307::begin() {
  i2c_init(I2C_PORT, I2C_FREQUENCY);
  gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
  gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_GPIO);
  gpio_pull_up(SCL_GPIO);
}

uint8_t read_register(uint8_t reg) {
  uint8_t buff;
  i2c_read_blocking(i2c0, DS1307_ADDRESS, &reg, 1, &buff, 1, false);
  return buff;
}

void write_register(uint8_t reg, uint8_t val) {
  uint8_t buffer[2] = {reg, val};
  i2c_write_blocking(i2c0, DS1307_ADDRESS, buffer, 2, true);
}

uint8_t RTC_DS1307::isrunning(void) { return !(read_register(0) >> 7); }

void RTC_DS1307::adjust(const DateTime &dt) {
  uint8_t buffer[8] = {0,
                       bin2bcd(dt.second()),
                       bin2bcd(dt.minute()),
                       bin2bcd(dt.hour()),
                       0,
                       bin2bcd(dt.day()),
                       bin2bcd(dt.month()),
                       bin2bcd(dt.year() - 2000U)};
  i2c_write_blocking(i2c0, DS1307_ADDRESS, buffer, 8, true);
}

DateTime RTC_DS1307::now() {
  uint8_t buffer[7];
  uint8_t data = 0x00;
  i2c_write_blocking(i2c0, DS1307_ADDRESS, &data, 1, false);
  i2c_read_blocking(i2c0, DS1307_ADDRESS, buffer, 7, false);

  return DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5]),
                  bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                  bcd2bin(buffer[0] & 0x7F));
}

Ds1307SqwPinMode RTC_DS1307::readSqwPinMode() {
  return static_cast<Ds1307SqwPinMode>(read_register(DS1307_CONTROL) & 0x93);
}

void RTC_DS1307::writeSqwPinMode(Ds1307SqwPinMode mode) {
  write_register(DS1307_CONTROL, mode);
}

void RTC_DS1307::readnvram(uint8_t *buf, uint8_t size, uint8_t address) {
  uint8_t addrByte = DS1307_NVRAM + address;
  i2c_read_blocking(I2C_PORT, addrByte, buf, size, false);
}

void RTC_DS1307::writenvram(uint8_t address, const uint8_t *buf, uint8_t size) {
  uint8_t addrByte = DS1307_NVRAM + address;
  i2c_write_blocking(I2C_PORT, addrByte, buf, size, true);
}

uint8_t RTC_DS1307::readnvram(uint8_t address) {
  uint8_t data;
  readnvram(&data, 1, address);
  return data;
}

void RTC_DS1307::writenvram(uint8_t address, uint8_t data) {
  writenvram(address, &data, 1);
}