#include "as5600.h"

AS5600::AS5600()
{
    // Set up I2C
    i2c_init(I2C_PORT, I2C_FREQUENCY);
    gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_GPIO);
    gpio_pull_up(SCL_GPIO);
}

/*
 * Function: getPosition
 * ----------------------------
 *   returns: the unscaled and unmodified angle from the RAW ANGLE register.
 */
uint16_t AS5600::getPosition()
{
    return getRawAngle();
}

/*
 * Function: getAngle
 * ----------------------------
 *   returns: the scaled output value available in the ANGLE register.
 */
uint16_t AS5600::getAngle()
{
    return _getRegisters2(_ANGLEAddressMSB, _ANGLEAddressLSB);
}

/*
 * Function: getRawAngle
 * ----------------------------
 *   returns: the unscaled and unmodified angle from the RAW ANGLE register.
 */
uint16_t AS5600::getRawAngle()
{
    return _getRegisters2(_RAWANGLEAddressMSB, _RAWANGLEAddressLSB);
}

/*
 * Function: getScaledAngle
 * ----------------------------
 *   returns: the raw angle as a value between 0 and 360 degrees.
 */
float AS5600::getScaledAngle()
{
    int ang_hi = _getRegister(_RAWANGLEAddressMSB);
    int ang_lo = _getRegister(_RAWANGLEAddressLSB);
    return ang_hi * 22.5 + ang_lo * 0.087890625;
}

/*
 * Function: getStatus
 * ----------------------------
 *   returns: The the value in the STATUS register. The STATUS register provides bits that indicate the current state of the AS5600.
 *
 *   register format: X X MD ML MH X X X
 *            
 *   MH: AGC minimum gain overflow, magnet too strong
 *   ML: AGC maximum gain overflow, magnet too weak
 *   MD: Magnet was detected
 *
 */
uint8_t AS5600::getStatus()
{
    return _getRegister(_STATUSAddress) & 0b00111000;
}

/*
 * Function: isMagnetTooStrong
 * ----------------------------
 *   returns: true if magnet is too close to AS5600.
 */
bool AS5600::isMagnetTooStrong()
{
    uint8_t _b = 0;
    _b = getStatus();
    if (_b & (1 << 5))
    {
        return true;
    }
    return false;
}

/*
 * Function: isMagnetDetected
 * ----------------------------
 *   returns: true if magnet is too far to AS5600.
 */
bool AS5600::isMagnetTooWeak()
{
    uint8_t _b = 0;
    _b = getStatus();
    if (_b & (1 << 4))
    {
        return true;
    }
    return false;
}

/*
 * Function: isMagnetDetected
 * ----------------------------
 *   returns: true if magnet is detected by AS5600.
 */
bool AS5600::isMagnetDetected()
{
    uint8_t _b = 0;
    _b = getStatus();
    if (_b & (1 << 3))
    {
        return true;
    }
    return false;
}

/*
 * Function: getGain
 * ----------------------------
 *   returns: the value contained in the Automatic Gain Control (AGC) register.
 *    
 *   In 5V operation, the AGC range is 0-255 
 *   In 3.3V operation, the AGC range is 0-128
 */
uint8_t AS5600::getGain()
{
    return _getRegister(_AGCAddress);
}

/*
 * Function: getMagnet
 * ----------------------------
 *   returns: getGain().
 */
uint8_t AS5600::getMagnet()
{
    return getGain();
}

/*
 * Function: getMagnitude
 * ----------------------------
 *   returns: the value contained in the MAGNITUDE REGISTER. The MAGNITUDE register indicates the magnitude value of the
 *            internal Coordinate Rotation Digital Computer (CORDIC). 
 */
uint16_t AS5600::getMagnitude()
{
    return _getRegisters2(_MAGNITUDEAddressMSB, _MAGNITUDEAddressLSB);
}

/*
 * Function: setPowerMode
 * ----------------------------
 *   powerMode: the desired power mode. Valid input values are 0, 1, 2, and 3, corresponding to Normal Mode, Low Power Mode 1, Low Power Mode 2, Low Power Mode 3.
 * 
 *   returns: boolean indicating is value was set. 
 */
bool AS5600::setPowerMode(uint8_t powerMode)
{
    if (powerMode != POWER_MODE_NORM && powerMode != POWER_MODE_LPM1 && powerMode != POWER_MODE_LPM2 && powerMode != POWER_MODE_LPM3)
    {
        return false;
    }

    uint8_t currentCONFLSB = _getRegister(_CONFAddressLSB);
    uint8_t writeCONFLSB = currentCONFLSB & 0b11111100 | powerMode;
    _writeRegister(_CONFAddressLSB, writeCONFLSB);

    currentCONFLSB = _getRegister(_CONFAddressLSB);

    return currentCONFLSB == writeCONFLSB;
}

/*
 * Function: setHysteresis
 * ----------------------------
 *   hysteresis: the desired hysteresis. Valid values are 0, 1, 2, and 3, corresponding to OFF, 1 LSB, 2 LSBs, 3 LSBs.
 * 
 *   returns: boolean indicating is value was set. 
 */
bool AS5600::setHysteresis(uint8_t hysteresis)
{
    if (hysteresis != HYSTERESIS_OFF && hysteresis != HYSTERESIS_1LSB && hysteresis != HYSTERESIS_2LSB && hysteresis != HYSTERESIS_3LSB)
    {
        return false;
    }

    uint8_t currentCONFLSB = _getRegister(_CONFAddressLSB);
    uint8_t writeCONFLSB = currentCONFLSB & 0b11110011 | (hysteresis << 2);
    _writeRegister(_CONFAddressLSB, writeCONFLSB);

    currentCONFLSB = _getRegister(_CONFAddressLSB);

    return currentCONFLSB == writeCONFLSB;
}

/*
 * Function: setOutputStage
 * ----------------------------
 *   outputStage: the desired outputStage. Valid values are 0, 1 and 2 
 * 
 *   returns: boolean indicating is value was set. 
 */
bool AS5600::setOutputStage(uint8_t outputStage)
{
    if (outputStage != OUTPUT_STAGE_ANALOG_FULL && outputStage != OUTPUT_STAGE_ANALOG_REDUCED && outputStage != OUTPUT_STAGE_DIGITAL_PWM)
    {
        return false;
    }

    uint8_t currentCONFLSB = _getRegister(_CONFAddressLSB);
    uint8_t writeCONFLSB = currentCONFLSB & 0b11001111 | (outputStage << 4);
    _writeRegister(_CONFAddressLSB, writeCONFLSB);

    currentCONFLSB = _getRegister(_CONFAddressLSB);

    return currentCONFLSB == writeCONFLSB;
}

/*
 * Function: setPWMFrequency
 * ----------------------------
 *   frequency: the desired PWM frequency for PWM output. Valid values are 115 Hz, 230 Hz, 460 Hz, 920 Hz.
 * 
 *   returns: boolean indicating is value was set. 
 */

bool AS5600::setPWMFrequency(uint8_t frequency)
{

    uint8_t currentCONFLSB = _getRegister(_CONFAddressLSB);
    uint8_t writeCONFLSB = currentCONFLSB & 0b00111111 | (frequency << 6);
    _writeRegister(_CONFAddressLSB, writeCONFLSB);

    currentCONFLSB = _getRegister(_CONFAddressLSB);

    return currentCONFLSB == writeCONFLSB;
}

/*
 * Function: setSlowFilter
 * ----------------------------
 *   outputStage: the desired outputStage. Valid values are 0, 1, 2.
 * 
 *   returns: boolean indicating is value was set. 
 */
bool AS5600::setSlowFilter(uint8_t slowFilter)
{
    if (slowFilter != SLOW_FILTER_16X && slowFilter != SLOW_FILTER_8X && slowFilter != SLOW_FILTER_4X && slowFilter != SLOW_FILTER_2X)
    {
        return false;
    }

    uint8_t currentCONFLSB = _getRegister(_CONFAddressLSB);
    uint8_t writeCONFLSB = currentCONFLSB & 0b11111100 | slowFilter;
    _writeRegister(_CONFAddressLSB, writeCONFLSB);

    currentCONFLSB = _getRegister(_CONFAddressMSB);

    return currentCONFLSB == writeCONFLSB;
}

uint8_t AS5600::getCONF()
{

    return _getRegister(_CONFAddressLSB);
}

/*
 * Function: setFastFilterThreshold
 * ----------------------------
 *   fastFilterThreshold: the desired fastFilterThreshold. Valid values are 0, 1, 2, 3, 4, 5, 6, 7
 * 
 *   returns: boolean indicating is value was set. 
 */
bool AS5600::setFastFilterThreshold(uint8_t fastFilterThreshold)
{
    if (fastFilterThreshold != FAST_FILTER_THRESHOLD_SLOW &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_6LSB &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_7LSB &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_9LSB &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_18LSB &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_21LSB &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_24LSB &&
        fastFilterThreshold != FAST_FILTER_THRESHOLD_10LSB)
    {
        return false;
    }

    uint8_t currentCONFLSB = _getRegister(_CONFAddressLSB);
    uint8_t writeCONFLSB = currentCONFLSB & 0b11100011 | (fastFilterThreshold << 3);
    _writeRegister(_CONFAddressLSB, writeCONFLSB);

    currentCONFLSB = _getRegister(_CONFAddressMSB);

    return currentCONFLSB == writeCONFLSB;
}

/*
 * Function: _getRegister
 * ----------------------------
 *   register1: register address
 *
 *   returns: the value within a register.
 */
uint8_t AS5600::_getRegister(uint8_t register1)
{
    uint8_t buff[1];

    // Wire.beginTransmission(_AS5600Address);
    // Wire.write(register1);
    // Wire.endTransmission();

    // Wire.requestFrom(_AS5600Address, 1);

    // while (Wire.available() == 0)
    // {
    // }
    // _b = Wire.read();
    sleep_ms(100);
    uint8_t reg = (uint8_t)register1;
    i2c_write_blocking(I2C_PORT, (uint8_t)_AS5600Address, &reg, 1, true);
    sleep_ms(100);
    i2c_read_blocking(I2C_PORT, (uint8_t)_AS5600Address, buff, 1, false);
    return buff[0];
}

/*
 * Function: _getRegisters2
 * ----------------------------
 *   registerMSB: register address of Most Significant uint8_t (MMSB)
 *   registerLSB: register address of Least Significant uint8_t (MMSB)
 *
 *   returns: the value of the 16 bit number stored in registerMSB and registerLSB.
 */
uint16_t AS5600::_getRegisters2(uint8_t registerMSB, uint8_t registerLSB)
{

    uint16_t _hi = 0, _lo = 0;
    _hi = _getRegister(registerMSB);
    _lo = _getRegister(registerLSB);
    return (_hi << 8) | (_lo);
}

/*
 * Function: _writeRegister
 * ----------------------------
 *   registerAddress: register address to write to
 *   value: value to write to register at registerAddress
 *
 *   returns: void
 */
void AS5600::_writeRegister(uint8_t registerAddress, uint8_t value)
{
    // Wire.beginTransmission(_AS5600Address);
    // Wire.write((uint8_t)registerAddress); //module function register address
    // Wire.write((uint8_t)value);           //data uint8_ts
    // Wire.endTransmission();
    uint8_t buf[] = {(uint8_t)registerAddress, (uint8_t)value};
    i2c_write_blocking(I2C_PORT, (uint8_t)_AS5600Address, buf, 2, false);
}
