#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define SDA_GPIO 8
#define SCL_GPIO 9
#define I2C_PORT i2c0
#define I2C_FREQUENCY 400000

// Register addresses
#define CSS811_STATUS 0x00
#define CSS811_MEAS_MODE 0x01
#define CSS811_ALG_RESULT_DATA 0x02
#define CSS811_RAW_DATA 0x03
#define CSS811_ENV_DATA 0x05
#define CSS811_NTC 0x06 // NTC compensation no longer supported
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

class CCS811Core {
public:
  typedef enum {
    CCS811_Stat_SUCCESS,
    CCS811_Stat_ID_ERROR,
    CCS811_Stat_I2C_ERROR,
    CCS811_Stat_INTERNAL_ERROR,
    CCS811_Stat_NUM,
    CCS811_Stat_GENERIC_ERROR
  } CCS811_Status_e;

  CCS811Core(uint8_t);
  ~CCS811Core() = default;

  void setI2CAddress(uint8_t address) { I2CAddress = address; }
  CCS811_Status_e beginCore(i2c_inst_t *i2cPort);
  CCS811_Status_e readRegister(uint8_t offset, uint8_t *outputPointer);
  CCS811_Status_e multiReadRegister(uint8_t offset, uint8_t *outputPointer,
                                    uint8_t length);

  CCS811_Status_e writeRegister(uint8_t offset, uint8_t dataToWrite);
  CCS811_Status_e multiWriteRegister(uint8_t offset, uint8_t *inputPointer,
                                     uint8_t length);

protected:
  i2c_inst_t *_i2cPort;
  uint8_t I2CAddress;
};

class CCS811 : public CCS811Core {
public:
  CCS811(uint8_t);
  CCS811();

  bool begin(i2c_inst_t *i2cPort = I2C_PORT);
  CCS811_Status_e beginWithStatus(i2c_inst_t *i2cPort = I2C_PORT);
  const char *statusString(CCS811_Status_e stat = CCS811_Stat_NUM);

  CCS811_Status_e readAlgorithmResults(void);
  bool checkForStatusError(void);
  bool dataAvailable(void);
  bool appValid(void);
  uint8_t getErrorRegister(void);
  uint16_t getBaseline(void);
  CCS811_Status_e setBaseline(uint16_t);
  CCS811_Status_e enableInterrupts(void);
  CCS811_Status_e disableInterrupts(void);
  CCS811_Status_e setDriveMode(uint8_t mode);
  CCS811_Status_e setEnvironmentalData(float relativeHumidity,
                                       float temperature);
  void setRefResistance(float);
  CCS811_Status_e readNTC(void);
  uint16_t getTVOC(void);
  uint16_t getCO2(void);
  float getResistance(void);
  float getTemperature(void);

private:
  float refResistance;
  float resistance;
  uint16_t tVOC;
  uint16_t CO2;
  uint16_t vrefCounts = 0;
  uint16_t ntcCounts = 0;
  float temperature;
};