
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

#define BH1750_ADDR 0x23
#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_DEFAULT_MTREG 69
#define BH1750_MTREG_MIN 31
#define BH1750_MTREG_MAX 254

class BH1750 {

public:
  enum Mode {
    // same as Power Down
    UNCONFIGURED = 0,
    // Measurement at 1 lux resolution. Measurement time is approx 120ms.
    CONTINUOUS_HIGH_RES_MODE = 0x10,
    // Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
    CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
    // Measurement at 4 lux resolution. Measurement time is approx 16ms.
    CONTINUOUS_LOW_RES_MODE = 0x13,
    // Measurement at 1 lux resolution. Measurement time is approx 120ms.
    ONE_TIME_HIGH_RES_MODE = 0x20,
    // Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
    ONE_TIME_HIGH_RES_MODE_2 = 0x21,
    // Measurement at 4 lux resolution. Measurement time is approx 16ms.
    ONE_TIME_LOW_RES_MODE = 0x23
  };

  BH1750(uint8_t addr = (uint8_t)BH1750_ADDR);
  bool begin(Mode mode = CONTINUOUS_HIGH_RES_MODE,
             uint8_t addr = (uint8_t)BH1750_ADDR, i2c_inst_t *i2c = nullptr);
  bool configure(Mode mode);
  bool setMTreg(uint8_t MTreg);
  bool measurementReady(bool maxWait = false);
  float readLightLevel();

private:
  uint8_t BH1750_I2CADDR;
  uint8_t BH1750_MTreg = (uint8_t)BH1750_DEFAULT_MTREG;
  const float BH1750_CONV_FACTOR = 1.2;
  Mode BH1750_MODE = UNCONFIGURED;
  i2c_inst_t *I2C;
  unsigned long lastReadTimestamp;
};
