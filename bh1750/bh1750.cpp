#include "bh1750.h"

BH1750::BH1750(uint8_t addr) {

  BH1750_I2CADDR = addr;
  I2C = I2C_PORT;
}
bool BH1750::begin(Mode mode, uint8_t addr, i2c_inst_t *i2c) {

  if (i2c) {
    I2C = i2c;
  }
  if (addr) {
    BH1750_I2CADDR = addr;
  }
  // Set up I2C
  i2c_init(I2C_PORT, I2C_FREQUENCY);
  gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
  gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_GPIO);
  gpio_pull_up(SCL_GPIO);

  return (configure(mode) && setMTreg(BH1750_DEFAULT_MTREG));
}

bool BH1750::configure(Mode mode) {
  uint8_t buff = (uint8_t)mode;
  switch (mode) {
  case BH1750::CONTINUOUS_HIGH_RES_MODE:
  case BH1750::CONTINUOUS_HIGH_RES_MODE_2:
  case BH1750::CONTINUOUS_LOW_RES_MODE:
  case BH1750::ONE_TIME_HIGH_RES_MODE:
  case BH1750::ONE_TIME_HIGH_RES_MODE_2:
  case BH1750::ONE_TIME_LOW_RES_MODE:
    i2c_write_blocking(I2C, BH1750_I2CADDR, &buff, 1, false);
    sleep_ms(10);
    break;

  default:
    printf("[BH1750] ERROR: Invalid mode");
    break;
  }
  BH1750_MODE = mode;
  return true;
}

bool BH1750::setMTreg(uint8_t MTreg) {
  if (MTreg < BH1750_MTREG_MIN || MTreg > BH1750_MTREG_MAX) {
    printf("[BH1750] ERROR: MTreg out of range");
    return false;
  }
  uint8_t buff = (uint8_t)(0b01000 << 3) | (MTreg >> 5);
  i2c_write_blocking(I2C, BH1750_I2CADDR, &buff, 1, false);
  buff = (uint8_t)(0b011 << 5) | (MTreg & 0b11111);
  i2c_write_blocking(I2C, BH1750_I2CADDR, &buff, 1, false);
  buff = BH1750_MODE;
  i2c_write_blocking(I2C, BH1750_I2CADDR, &buff, 1, false);
  sleep_ms(10);
  BH1750_MTreg = MTreg;

  return true;
}

bool BH1750::measurementReady(bool maxWait) {
  unsigned long delaytime = 0;
  switch (BH1750_MODE) {
  case BH1750::CONTINUOUS_HIGH_RES_MODE:
  case BH1750::CONTINUOUS_HIGH_RES_MODE_2:
  case BH1750::ONE_TIME_HIGH_RES_MODE:
  case BH1750::ONE_TIME_HIGH_RES_MODE_2:
    maxWait ? delaytime = (180 * BH1750_MTreg / (uint8_t)BH1750_DEFAULT_MTREG)
            : delaytime = (120 * BH1750_MTreg / (uint8_t)BH1750_DEFAULT_MTREG);
    break;
  case BH1750::CONTINUOUS_LOW_RES_MODE:
  case BH1750::ONE_TIME_LOW_RES_MODE:
    // Send mode to sensor
    maxWait ? delaytime = (24 * BH1750_MTreg / (uint8_t)BH1750_DEFAULT_MTREG)
            : delaytime = (16 * BH1750_MTreg / (uint8_t)BH1750_DEFAULT_MTREG);
    break;
  default:
    break;
  }
  unsigned long currentTimestamp = time_us_64();
  if (currentTimestamp - lastReadTimestamp >= delaytime) {
    return true;
  } else
    return false;
}

float BH1750::readLightLevel() {

  if (BH1750_MODE == UNCONFIGURED) {
    printf("[BH1750] Device is not configured!");
    return -2.0;
  }

  float level = -1.0;
  uint8_t data[2];
  i2c_read_blocking(I2C, BH1750_I2CADDR, data, 2, false);
  level = (uint16_t)data[0] << 8 | data[1];
  lastReadTimestamp = time_us_64();

  if (level != -1.0) {
#ifdef BH1750_DEBUG
    printf("[BH1750] Raw value: ");
    printf("%f", level);
#endif

    if (BH1750_MTreg != BH1750_DEFAULT_MTREG) {
      level *= (float)((uint8_t)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg);
// Print MTreg factor if debug enabled
#ifdef BH1750_DEBUG
      printf("[BH1750] MTreg factor: ");
      printf("%f",
             (float)((uint8_t)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg));
#endif
    }
    if (BH1750_MODE == BH1750::ONE_TIME_HIGH_RES_MODE_2 ||
        BH1750_MODE == BH1750::CONTINUOUS_HIGH_RES_MODE_2) {
      level /= 2;
    }
    // Convert raw value to lux
    level /= BH1750_CONV_FACTOR;

#ifdef BH1750_DEBUG
    printf("[BH1750] Converted float value: ");
    printf("%f", level);
#endif
  }
  return level;
}