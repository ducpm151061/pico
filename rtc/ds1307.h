#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define SECONDS_PER_DAY 86400L ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000                                              \
  946684800 ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

enum Ds1307SqwPinMode {
  DS1307_OFF = 0x00,            // Low
  DS1307_ON = 0x80,             // High
  DS1307_SquareWave1HZ = 0x10,  // 1Hz square wave
  DS1307_SquareWave4kHz = 0x11, // 4kHz square wave
  DS1307_SquareWave8kHz = 0x12, // 8kHz square wave
  DS1307_SquareWave32kHz = 0x13 // 32kHz square wave
};

class DateTime {
public:
  DateTime(uint32_t t = SECONDS_FROM_1970_TO_2000);
  DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0,
           uint8_t min = 0, uint8_t sec = 0);
  DateTime(const DateTime &copy);
  DateTime(const char *date, const char *time);
  DateTime(const char *iso8601date);
  bool isValid() const;
  char *toString(char *buffer) const;
  uint16_t year() const { return 2000U + yOff; }
  uint8_t month() const { return m; }
  uint8_t day() const { return d; }
  uint8_t hour() const { return hh; }

  uint8_t twelveHour() const;
  uint8_t isPM() const { return hh >= 12; }
  uint8_t minute() const { return mm; }
  uint8_t second() const { return ss; }
  uint8_t dayOfTheWeek() const;
  /* 32-bit times as seconds since 2000-01-01. */
  uint32_t secondstime() const;
  /* 32-bit times as seconds since 1970-01-01. */
  uint32_t unixtime(void) const;
  /*!
      Format of the ISO 8601 timestamp generated by `timestamp()`. Each
      option corresponds to a `toString()` format as follows:
  */
  enum timestampOpt {
    TIMESTAMP_FULL, //!< `YYYY-MM-DDThh:mm:ss`
    TIMESTAMP_TIME, //!< `hh:mm:ss`
    TIMESTAMP_DATE  //!< `YYYY-MM-DD`
  };
  String timestamp(timestampOpt opt = TIMESTAMP_FULL) const;

  DateTime operator+(const TimeSpan &span) const;
  DateTime operator-(const TimeSpan &span) const;
  TimeSpan operator-(const DateTime &right) const;
  bool operator<(const DateTime &right) const;
  bool operator>(const DateTime &right) const { return right < *this; }
  bool operator<=(const DateTime &right) const { return !(*this > right); }
  bool operator>=(const DateTime &right) const { return !(*this < right); }
  bool operator==(const DateTime &right) const;
  bool operator!=(const DateTime &right) const { return !(*this == right); }

protected:
  uint8_t yOff; ///< Year offset from 2000
  uint8_t m;    ///< Month 1-12
  uint8_t d;    ///< Day 1-31
  uint8_t hh;   ///< Hours 0-23
  uint8_t mm;   ///< Minutes 0-59
  uint8_t ss;   ///< Seconds 0-59
};
class TimeSpan {
public:
  TimeSpan(int32_t seconds = 0);
  TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
  TimeSpan(const TimeSpan &copy);
  int16_t days() const { return _seconds / 86400L; }
  int8_t hours() const { return _seconds / 3600 % 24; }
  int8_t minutes() const { return _seconds / 60 % 60; }
  int8_t seconds() const { return _seconds % 60; }
  int32_t totalseconds() const { return _seconds; }

  TimeSpan operator+(const TimeSpan &right) const;
  TimeSpan operator-(const TimeSpan &right) const;

protected:
  int32_t _seconds; ///< Actual TimeSpan value is stored as seconds
};


class RTC_DS1307 {
public:
  void begin();
  uint8_t read_register(uint8_t reg);
  void write_register(uint8_t reg, uint8_t val);
  void adjust(const DateTime &dt);
  uint8_t isrunning(void);
  DateTime now();
  Ds1307SqwPinMode readSqwPinMode();
  void writeSqwPinMode(Ds1307SqwPinMode mode);
  uint8_t readnvram(uint8_t address);
  void readnvram(uint8_t *buf, uint8_t size, uint8_t address);
  void writenvram(uint8_t address, uint8_t data);
  void writenvram(uint8_t address, const uint8_t *buf, uint8_t size);
  static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
  static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }
};
class RTC_Millis {
public:
  void begin(const DateTime &dt) { adjust(dt); }
  void adjust(const DateTime &dt);
  DateTime now();

protected:
  uint32_t lastUnix;
  uint32_t lastMillis;
};

class RTC_Micros {
public:
  void begin(const DateTime &dt) { adjust(dt); }
  void adjust(const DateTime &dt);
  void adjustDrift(int ppm);
  DateTime now();

protected:
  uint32_t microsPerSecond = 1000000;
  uint32_t lastUnix;
  uint32_t lastMicros;
};