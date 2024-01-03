#include "ds1307.h"
#include "pico/util/datetime.h"
#include <stdio.h>

static uint8_t decToBcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }
static uint8_t bcdToDec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; i++)
    days += daysInMonth[i - 1];
  if (m > 2 && y % 4 == 0)
    days++;
  return days + 365 * y + (y + 3) / 4 - 1;
}

static inline long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24L + h) * 60 + m) * 60 + s;
}

static uint8_t dOfW(uint16_t y, uint8_t m, uint8_t d) {
  uint16_t day = date2days(y, m, d);
  return (day + 6) % 7;
}

uint8_t DS1307::read(uint8_t addr) {
  uint8_t reg = addr + 0x08;
  i2c_write_blocking(_port, DS1307_I2C_ADD, &reg, 1, false);
  uint8_t data;
  i2c_read_blocking(_port, DS1307_I2C_ADD, &data, 1, false);
  return data;
}

void DS1307::write(uint8_t addr, uint8_t val) {
  uint8_t data[2];
  data[0] = addr + 0x08;
  data[1] = val;
  i2c_write_blocking(_port, DS1307_I2C_ADD, data, 2, false);
}

long DS1307::getEpoch() {
  uint16_t days = date2days(year, month, dayOfMonth);
  uint32_t t = time2long(days, hour, minute, second);
  t += SECONDS_FROM_1970_TO_2000;
  return t;
}

void DS1307::DSgetTime(char *time) {
  char datetime_buf[256];
  datetime_t t = {.year = year + 2000,
                  .month = month,
                  .day = dayOfMonth,
                  .dotw = dayOfWeek,
                  .hour = hour,
                  .min = minute,
                  .sec = second};
  datetime_to_str(time, sizeof(datetime_buf), &t);
}

void DS1307::begin(i2c_inst_t *port) {
  _port = port;
  i2c_init(_port, 400 * 1000);
}

void DS1307::DSread() {
  // Reset the register pointer
  uint8_t reg = 0x00;
  i2c_write_blocking(_port, DS1307_I2C_ADD, &reg, 1, false);

  // Request 7 bytes of data
  uint8_t data[7];
  i2c_read_blocking(_port, DS1307_I2C_ADD, data, 7, false);

  second = bcdToDec(data[0] & 0x7f);
  minute = bcdToDec(data[1]);
  hour = bcdToDec(data[2] & 0x3f);
  dayOfWeek = bcdToDec(data[3]);
  dayOfMonth = bcdToDec(data[4]);
  month = bcdToDec(data[5]);
  year = bcdToDec(data[6]);
  dayOfWeek %= 7;
}

void DS1307::DSwrite() {
  uint8_t data[8];
  data[0] = 0x00;
  data[1] = decToBcd(second);
  data[2] = decToBcd(minute);
  data[3] = decToBcd(hour);
  data[4] = decToBcd(dayOfWeek);
  data[5] = decToBcd(dayOfMonth);
  data[6] = decToBcd(month);
  data[7] = decToBcd(year);

  i2c_write_blocking(_port, DS1307_I2C_ADD, data, 8, false);
}

void DS1307::DSadjust(uint8_t _hour, uint8_t _minute, uint8_t _second,
                      uint16_t _year, uint8_t _month, uint8_t _day) {
  // assign variables
  hour = _hour;
  minute = _minute;
  second = _second;
  year = _year - 2000;
  month = _month;
  dayOfMonth = _day;
  uint16_t day = date2days(year, month, dayOfMonth);
  dayOfWeek = (day + 6) % 7;
  DSwrite();
}

void DS1307::DSadjust(uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970
  second = t % 60;
  t /= 60;
  minute = t % 60;
  t /= 60;
  hour = t % 24;
  uint16_t days = t / 24;
  uint8_t leap;
  for (year = 0;; year++) {
    leap = year % 4 == 0;
    if (days < 365u + leap)
      break;
    days -= 365 + leap;
  }

  for (month = 1;; month++) {
    uint8_t daysPerMonth = daysInMonth[month - 1];
    if (leap && month == 2)
      daysPerMonth++;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }

  dayOfMonth = days + 1;
  uint16_t day = date2days(year, month, dayOfMonth);
  dayOfWeek = (day + 6) % 7;
  DSwrite();
}