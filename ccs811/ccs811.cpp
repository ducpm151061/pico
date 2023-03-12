#include "ccs811.h"
#include <math.h>

CCS811Core::CCS811Core(uint8_t inputArg) : I2CAddress(inputArg) {}

CCS811Core::CCS811_Status_e CCS811Core::beginCore(i2c_inst_t *i2cPort) {
  CCS811Core::CCS811_Status_e returnError = CCS811_Stat_SUCCESS;

  _i2cPort = i2cPort;
  // Set up I2C
  i2c_init(_i2cPort, I2C_FREQUENCY);
  gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
  gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_GPIO);
  gpio_pull_up(SCL_GPIO);

  // Spin for a few ms
  volatile uint8_t temp = 0;
  for (uint16_t i = 0; i < 10000; i++) {
    temp++;
  }

  // Check the ID register to determine if the operation was a success.
  uint8_t readCheck;
  readCheck = 0;
  returnError = readRegister(CSS811_HW_ID, &readCheck);

  if (returnError != CCS811_Stat_SUCCESS)
    return returnError;

  if (readCheck != 0x81) {
    returnError = CCS811_Stat_ID_ERROR;
  }

  return returnError;
}

CCS811Core::CCS811_Status_e CCS811Core::readRegister(uint8_t offset,
                                                     uint8_t *outputPointer) {
  uint8_t numBytes = 1;
  CCS811Core::CCS811_Status_e returnError = CCS811_Stat_SUCCESS;

  i2c_write_blocking(_i2cPort, I2CAddress, &offset, 1, false);
  sleep_ms(10);
  i2c_read_blocking(_i2cPort, I2CAddress, outputPointer, numBytes, false);
  sleep_ms(10);

  return returnError;
}

CCS811Core::CCS811_Status_e
CCS811Core::multiReadRegister(uint8_t offset, uint8_t *outputPointer,
                              uint8_t length) {
  CCS811Core::CCS811_Status_e returnError = CCS811_Stat_SUCCESS;

  // define pointer that will point to the external space
  uint8_t i = 0;
  uint8_t c = 0;
  // Set the address
  i2c_write_blocking(_i2cPort, I2CAddress, &offset, 1, false);
  sleep_ms(10);
  i2c_read_blocking(_i2cPort, I2CAddress, outputPointer, length, false);
  sleep_ms(10);

  return returnError;
}

CCS811Core::CCS811_Status_e CCS811Core::writeRegister(uint8_t offset,
                                                      uint8_t dataToWrite) {
  CCS811Core::CCS811_Status_e returnError = CCS811_Stat_SUCCESS;
  uint8_t buff[] = {offset, dataToWrite};
  i2c_write_blocking(_i2cPort, I2CAddress, buff, 2, false);
  sleep_ms(10);
  return returnError;
}

CCS811Core::CCS811_Status_e
CCS811Core::multiWriteRegister(uint8_t offset, uint8_t *inputPointer,
                               uint8_t length) {
  CCS811Core::CCS811_Status_e returnError = CCS811_Stat_SUCCESS;
  uint8_t buff[length + 1];
  buff[0] = offset;
  for (uint8_t i = 0; i < length; i++) {
    buff[i + 1] = inputPointer[i];
  }
  i2c_write_blocking(_i2cPort, I2CAddress, buff, length + 1, false);
  sleep_ms(10);
  return returnError;
}

CCS811::CCS811(uint8_t inputArg) : CCS811Core(inputArg) {
  refResistance = 10000;
  resistance = 0;
  temperature = 0;
  tVOC = 0;
  CO2 = 0;
}

CCS811::CCS811() : CCS811(0) {}
bool CCS811::begin(i2c_inst_t *i2cPort) {
  if (beginWithStatus(i2cPort) == CCS811_Stat_SUCCESS)
    return true;
  return false;
}

CCS811Core::CCS811_Status_e CCS811::beginWithStatus(i2c_inst_t *i2cPort) {
  uint8_t data[4] = {0x11, 0xE5, 0x72, 0x8A}; // Reset key
  CCS811Core::CCS811_Status_e returnError =
      CCS811_Stat_SUCCESS; // Default error state

  // restart the core
  returnError = beginCore(i2cPort);

  if (returnError != CCS811_Stat_SUCCESS)
    return returnError;

  // Reset the device
  multiWriteRegister(CSS811_SW_RESET, data, 4);

  // Tclk = 1/16MHz = 0x0000000625
  // 0.001 s / tclk = 16000 counts
  volatile uint8_t temp = 0;

  for (uint32_t i = 0; i < 200000; i++) // Spin for a good while
  {
    temp++;
  }

  if (checkForStatusError() == true)
    return CCS811_Stat_INTERNAL_ERROR;

  if (appValid() == false)
    return CCS811_Stat_INTERNAL_ERROR;
  uint8_t buf = CSS811_APP_START;
  i2c_write_blocking(i2cPort, I2CAddress, &buf, 1, false);
  returnError = setDriveMode(1);
  return returnError;
}

CCS811Core::CCS811_Status_e CCS811::readAlgorithmResults(void) {
  uint8_t data[4];
  CCS811Core::CCS811_Status_e returnError =
      multiReadRegister(CSS811_ALG_RESULT_DATA, data, 4);
  if (returnError != CCS811_Stat_SUCCESS)
    return returnError;
  // Data ordered:
  // co2MSB, co2LSB, tvocMSB, tvocLSB

  CO2 = ((uint16_t)data[0] << 8) | data[1];
  tVOC = ((uint16_t)data[2] << 8) | data[3];
  return CCS811_Stat_SUCCESS;
}

// Checks to see if error bit is set
bool CCS811::checkForStatusError(void) {
  uint8_t value;
  // return the status bit
  readRegister(CSS811_STATUS, &value);
  return (value & 1 << 0);
}

// Checks to see if DATA_READ flag is set in the status register
bool CCS811::dataAvailable(void) {
  uint8_t value;
  CCS811Core::CCS811_Status_e returnError = readRegister(CSS811_STATUS, &value);
  if (returnError != CCS811_Stat_SUCCESS) {
    return 0;
  } else {
    return (value & 1 << 3);
  }
}

// Checks to see if APP_VALID flag is set in the status register
bool CCS811::appValid(void) {
  uint8_t value;
  CCS811Core::CCS811_Status_e returnError = readRegister(CSS811_STATUS, &value);
  if (returnError != CCS811_Stat_SUCCESS) {
    return 0;
  } else {
    return (value & 1 << 4);
  }
}

uint8_t CCS811::getErrorRegister(void) {
  uint8_t value;

  CCS811Core::CCS811_Status_e returnError =
      readRegister(CSS811_ERROR_ID, &value);
  if (returnError != CCS811_Stat_SUCCESS) {
    return 0xFF;
  } else {
    return value;
  }
}

uint16_t CCS811::getBaseline(void) {
  uint8_t data[2];
  CCS811Core::CCS811_Status_e returnError =
      multiReadRegister(CSS811_BASELINE, data, 2);

  unsigned int baseline = ((uint16_t)data[0] << 8) | data[1];
  if (returnError != CCS811_Stat_SUCCESS) {
    return 0;
  } else {
    return (baseline);
  }
}

CCS811Core::CCS811_Status_e CCS811::setBaseline(uint16_t input) {
  uint8_t data[2];
  data[0] = (input >> 8) & 0x00FF;
  data[1] = input & 0x00FF;

  CCS811Core::CCS811_Status_e returnError =
      multiWriteRegister(CSS811_BASELINE, data, 2);

  return returnError;
}

// Enable the nINT signal
CCS811Core::CCS811_Status_e CCS811::enableInterrupts(void) {
  uint8_t value;
  CCS811Core::CCS811_Status_e returnError =
      readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there
  if (returnError != CCS811_Stat_SUCCESS)
    return returnError;
  value |= (1 << 3); // Set INTERRUPT bit
  writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

// Disable the nINT signal
CCS811Core::CCS811_Status_e CCS811::disableInterrupts(void) {
  uint8_t value;
  CCS811Core::CCS811_Status_e returnError =
      readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there
  if (returnError != CCS811_Stat_SUCCESS)
    return returnError;
  value &= ~(1 << 3); // Clear INTERRUPT bit
  returnError = writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

// Mode 0 = Idle
// Mode 1 = read every 1s
// Mode 2 = every 10s
// Mode 3 = every 60s
// Mode 4 = RAW mode
CCS811Core::CCS811_Status_e CCS811::setDriveMode(uint8_t mode) {
  if (mode > 4)
    mode = 4; // sanitize input

  uint8_t value;
  CCS811Core::CCS811_Status_e returnError =
      readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there
  if (returnError != CCS811_Stat_SUCCESS)
    return returnError;
  value &= ~(0b00000111 << 4); // Clear DRIVE_MODE bits
  value |= (mode << 4);        // Mask in mode
  returnError = writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

CCS811Core::CCS811_Status_e CCS811::setEnvironmentalData(float relativeHumidity,
                                                         float temperature) {
  // Check for invalid temperatures
  if ((temperature < -25) || (temperature > 50))
    return CCS811_Stat_GENERIC_ERROR;

  // Check for invalid humidity
  if ((relativeHumidity < 0) || (relativeHumidity > 100))
    return CCS811_Stat_GENERIC_ERROR;

  uint32_t rH = relativeHumidity * 1000; // 42.348 becomes 42348
  uint32_t temp = temperature * 1000;    // 23.2 becomes 23200

  uint8_t envData[4];

  envData[0] = (rH + 250) / 500;
  envData[1] = 0; // CCS811 only supports increments of 0.5 so bits 7-0 will
                  // always be zero

  temp += 25000; // Add the 25C offset
  envData[2] = (temp + 250) / 500;
  envData[3] = 0;

  CCS811Core::CCS811_Status_e returnError =
      multiWriteRegister(CSS811_ENV_DATA, envData, 4);
  return returnError;
}

uint16_t CCS811::getTVOC(void) { return tVOC; }

uint16_t CCS811::getCO2(void) { return CO2; }

void CCS811::setRefResistance(float input) { refResistance = input; }

CCS811Core::CCS811_Status_e CCS811::readNTC(void) {
  uint8_t data[4];
  CCS811Core::CCS811_Status_e returnError =
      multiReadRegister(CSS811_NTC, data, 4);

  vrefCounts = ((uint16_t)data[0] << 8) | data[1];
  ntcCounts = ((uint16_t)data[2] << 8) | data[3];
  resistance = ((float)ntcCounts * refResistance / (float)vrefCounts);

  temperature = log((long)resistance);
  temperature =
      1 / (0.001129148 + (0.000234125 * temperature) +
           (0.0000000876741 * temperature * temperature * temperature));
  temperature = temperature - 273.15; // Convert Kelvin to Celsius

  return returnError;
}

float CCS811::getResistance(void) { return resistance; }

float CCS811::getTemperature(void) { return temperature; }

const char *CCS811::statusString(CCS811_Status_e stat) {
  CCS811_Status_e val;
  if (stat == CCS811_Stat_NUM) {
    val = stat;
  } else {
    val = stat;
  }

  switch (val) {
  case CCS811_Stat_SUCCESS:
    return "All is well.";
    break;
  case CCS811_Stat_ID_ERROR:
    return "ID Error";
    break;
  case CCS811_Stat_I2C_ERROR:
    return "I2C Error";
    break;
  case CCS811_Stat_INTERNAL_ERROR:
    return "Internal Error";
    break;
  case CCS811_Stat_GENERIC_ERROR:
    return "Generic Error";
    break;
  default:
    return "Unknown Status";
    break;
  }
  return "None";
}