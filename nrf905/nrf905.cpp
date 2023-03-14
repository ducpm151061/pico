#include "nrf905.h"

static const uint8_t config[] = {
    NRF905_CMD_W_CONFIG,
    NRF905_CHANNEL,
    NRF905_AUTO_RETRAN | NRF905_LOW_RX | NRF905_PWR | NRF905_BAND |
        ((NRF905_CHANNEL >> 8) & 0x01),
    (NRF905_ADDR_SIZE_TX << 4) | NRF905_ADDR_SIZE_RX,
    NRF905_PAYLOAD_SIZE_RX,
    NRF905_PAYLOAD_SIZE_TX,
    (uint8_t)NRF905_ADDRESS,
    (uint8_t)(NRF905_ADDRESS >> 8),
    (uint8_t)(NRF905_ADDRESS >> 16),
    (uint8_t)(NRF905_ADDRESS >> 24),
    NRF905_CRC | NRF905_CLK_FREQ | NRF905_OUTCLK};

inline uint8_t nRF905::cselect() {
  asm volatile("nop \n nop \n nop");
  gpio_put(csn, 0);
  asm volatile("nop \n nop \n nop");
  return 0;
}

inline uint8_t nRF905::cdeselect() {
  asm volatile("nop \n nop \n nop");
  gpio_put(csn, 1);
  asm volatile("nop \n nop \n nop");
  return 0;
}

// Can be in any mode to write registers, but standby or power-down is
// recommended
#define CHIPSELECT() for (uint8_t _cs = cselect(); _cs; _cs = cdeselect())

uint8_t nRF905::readConfigRegister(uint8_t reg) {
  cselect();
  uint8_t val = 0;
  uint8_t buff = NRF905_CMD_R_CONFIG | reg;
  spi_write_blocking(spi, &buff, 1);
  sleep_ms(10);
  spi_read_blocking(spi, NRF905_CMD_NOP, &val, 1);
  cdeselect();
  sleep_ms(10);
  return val;
}

void nRF905::writeConfigRegister(uint8_t reg, uint8_t val) {
  cselect();
  uint8_t buff = NRF905_CMD_W_CONFIG | reg;
  spi_write_blocking(spi, &buff, 1);
  sleep_ms(10);
  spi_write_blocking(spi, &val, 1);
  cdeselect();
  sleep_ms(10);
}

void nRF905::setConfigReg1(uint8_t val, uint8_t mask, uint8_t reg) {
  // TODO atomic read/write?
  writeConfigRegister(reg,
                      (readConfigRegister(NRF905_REG_CONFIG1) & mask) | val);
}

void nRF905::setConfigReg2(uint8_t val, uint8_t mask, uint8_t reg) {
  // TODO atomic read/write?
  writeConfigRegister(reg,
                      (readConfigRegister(NRF905_REG_CONFIG2) & mask) | val);
}

void nRF905::defaultConfig() {
  // Should be in standby mode
  // Set control registers
  cselect();
  sleep_ms(10);
  spi_write_blocking(spi, config, sizeof(config));
  cdeselect();
  sleep_ms(10);

  cselect();
  sleep_ms(10);
  uint8_t data = NRF905_CMD_W_TX_ADDRESS;
  uint8_t buff[] = {0xE7, 0xE7, 0xE7, 0xE7};
  spi_write_blocking(spi, &data, 1);
  spi_write_blocking(spi, buff, sizeof(buff));
  cdeselect();
  sleep_ms(10);

  // Clear transmit payload
  // TODO is this really needed?
  cselect();
  sleep_ms(10);
  data = NRF905_CMD_W_TX_PAYLOAD;
  spi_write_blocking(spi, &data, 1);
  for (uint8_t i = 0; i < NRF905_MAX_PAYLOAD; i++)
    spi_write_blocking(spi, 0x00, 1);
  cdeselect();
  sleep_ms(10);

  if (pwr == NRF905_PIN_UNUSED) {
    cselect();
    sleep_ms(10);
    data = NRF905_CMD_R_RX_PAYLOAD;
    spi_write_blocking(spi, &data, 1);
    // Clear DR by reading receive payload
    for (uint8_t i = 0; i < 10; i++) {
      data = NRF905_CMD_NOP;
      spi_write_blocking(spi, &data, 1);
    }
    cdeselect();
    sleep_ms(10);
  }
}

// NOTE: SPI registers can still be accessed when in power-down mode
inline void nRF905::powerOn(bool val) {
  if (pwr != NRF905_PIN_UNUSED)
    gpio_put(pwr, val ? 1 : 0);
}

inline void nRF905::standbyMode(bool val) {
  if (trx != NRF905_PIN_UNUSED)
    gpio_put(trx, val ? 0 : 1);
}

inline void nRF905::txMode(bool val) {
  if (tx != NRF905_PIN_UNUSED)
    gpio_put(tx, val ? 1 : 0);
}

void nRF905::setAddress(uint32_t address, uint8_t cmd) {
  cselect();
  sleep_ms(10);
  spi_write_blocking(spi, &cmd, 1);
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t data = address >> (8 * i);
    spi_write_blocking(spi, &data, 1);
  }
  cdeselect();
  sleep_ms(10);
}

uint8_t nRF905::readStatus() {
  uint8_t status = 0;
  CHIPSELECT()
  uint8_t status;
  spi_read_blocking(spi, NRF905_CMD_NOP, &status, 1);
  return status;
}

// TODO not used
/*
bool nRF905::dataReady()
{
        if(dr_mode == 1)
                return (readStatus() & (1<<NRF905_STATUS_DR));
        return gpio_put(dr);
}
*/

bool nRF905::addressMatched() {
  if (am == NRF905_PIN_UNUSED)
    return (readStatus() & (1 << NRF905_STATUS_AM));
  return gpio_get(am);
}

nRF905::nRF905() {}

void nRF905::begin(spi_inst_t *spi, uint32_t spiClock, int csn, int trx, int tx,
                   int pwr, int cd, int dr, int am,
                   void (*callback_interrupt_dr)(),
                   void (*callback_interrupt_am)()) {
  this->spi = spi;
  // this->spiSettings = SPISettings(spiClock, MSBFIRST, SPI_MODE0);

  this->csn = csn;
  this->trx = trx;
  this->tx = tx;
  this->pwr = pwr;
  this->cd = cd;
  this->dr = dr;
  this->am = am;

  gpio_put(csn, 1);
  gpio_set_function(csn, GPIO_FUNC_SPI);
  if (trx != NRF905_PIN_UNUSED)
    gpio_set_function(trx, GPIO_FUNC_SPI);
  if (tx != NRF905_PIN_UNUSED)
    gpio_set_function(tx, GPIO_FUNC_SPI);
  if (pwr != NRF905_PIN_UNUSED)
    gpio_set_function(pwr, GPIO_FUNC_SPI);

  powerOn(false);
  standbyMode(true);
  txMode(false);
  sleep_ms(3);
  defaultConfig();

  // if (dr == NRF905_PIN_UNUSED || callback_interrupt_dr == NULL)
  //   polledMode = true;
  // else {
  //   attachInterrupt(digitalPinToInterrupt(dr), callback_interrupt_dr,
  //   RISING); if (am != NRF905_PIN_UNUSED && callback_interrupt_am != NULL)
  //     attachInterrupt(digitalPinToInterrupt(am), callback_interrupt_am,
  //     CHANGE);
  // }
}

void nRF905::events(void (*onRxComplete)(nRF905 *device),
                    void (*onRxInvalid)(nRF905 *device),
                    void (*onTxComplete)(nRF905 *device),
                    void (*onAddrMatch)(nRF905 *device)) {
  this->onRxComplete = onRxComplete;
  this->onRxInvalid = onRxInvalid;
  this->onTxComplete = onTxComplete;
  this->onAddrMatch = onAddrMatch;
}

void nRF905::setChannel(uint16_t channel) {
  if (channel > 511)
    channel = 511;

  // TODO atomic read/write?
  cselect();
  uint8_t reg = (readConfigRegister(NRF905_REG_CONFIG1) & NRF905_MASK_CHANNEL) |
                (channel >> 8);
  uint8_t data = NRF905_CMD_W_CONFIG | NRF905_REG_CONFIG1;
  spi_write_blocking(spi, &data, 1);
  sleep_ms(10);
  data = (uint8_t)channel;
  spi_write_blocking(spi, &data, 1);
  sleep_ms(10);

  spi_write_blocking(spi, &reg, 1);
  sleep_ms(10);

  cdeselect();
  sleep_ms(10);
}

void nRF905::setBand(nRF905_band_t band) {
  // TODO atomic read/write?
  uint8_t data = NRF905_CMD_W_CONFIG | NRF905_REG_CONFIG1;
  uint8_t reg =
      (readConfigRegister(NRF905_REG_CONFIG1) & NRF905_MASK_BAND) | band;
  cselect();
  spi_write_blocking(spi, &data, 1);
  spi_write_blocking(spi, &reg, 1);
  sleep_ms(10);
  cdeselect();
  sleep_ms(10);
}

void nRF905::setAutoRetransmit(bool val) {
  setConfigReg1(val ? NRF905_AUTO_RETRAN_ENABLE : NRF905_AUTO_RETRAN_DISABLE,
                NRF905_MASK_AUTO_RETRAN, NRF905_REG_AUTO_RETRAN);
}

void nRF905::setLowRxPower(bool val) {
  setConfigReg1(val ? NRF905_LOW_RX_ENABLE : NRF905_LOW_RX_DISABLE,
                NRF905_MASK_LOW_RX, NRF905_REG_LOW_RX);
}

void nRF905::setTransmitPower(nRF905_pwr_t val) {
  setConfigReg1(val, NRF905_MASK_PWR, NRF905_REG_PWR);
}

void nRF905::setCRC(nRF905_crc_t val) {
  setConfigReg2(val, NRF905_MASK_CRC, NRF905_REG_CRC);
}

void nRF905::setClockOut(nRF905_outclk_t val) {
  setConfigReg2(val, NRF905_MASK_OUTCLK, NRF905_REG_OUTCLK);
}

void nRF905::setPayloadSize(uint8_t sizeTX, uint8_t sizeRX) {
  if (sizeTX > NRF905_MAX_PAYLOAD)
    sizeTX = NRF905_MAX_PAYLOAD;

  if (sizeRX > NRF905_MAX_PAYLOAD)
    sizeRX = NRF905_MAX_PAYLOAD;
  cselect();
  sleep_ms(10);
  uint8_t data = NRF905_CMD_W_CONFIG | NRF905_REG_TX_PAYLOAD_SIZE;
  spi_write_blocking(spi, &data, 1);
  spi_write_blocking(spi, &sizeTX, 1);
  spi_write_blocking(spi, &sizeRX, 1);
  sleep_ms(10);
  cdeselect();
}

void nRF905::setAddressSize(uint8_t sizeTX, uint8_t sizeRX) {
  if (sizeTX != 1 && sizeTX != 4)
    sizeTX = 4;

  if (sizeRX != 1 && sizeRX != 4)
    sizeRX = 4;
  cselect();
  sleep_ms(10);
  uint8_t data = NRF905_CMD_W_CONFIG | NRF905_REG_ADDR_WIDTH;
  spi_write_blocking(spi, &data, 1);
  data = (sizeTX << 4) | sizeRX;
  spi_write_blocking(spi, &data, 1);
  sleep_ms(10);
  cdeselect();
}

bool nRF905::receiveBusy() { return addressMatched(); }

bool nRF905::airwayBusy() {
  if (cd != NRF905_PIN_UNUSED)
    return gpio_get(cd);
  return false;
}

void nRF905::setListenAddress(uint32_t address) {
  setAddress(address, NRF905_CMD_W_CONFIG | NRF905_REG_RX_ADDRESS);
}

void nRF905::write(uint32_t sendTo, uint8_t *data, uint8_t len) {
  setAddress(sendTo, NRF905_CMD_W_TX_ADDRESS);

  if (len > 0 && data != NULL) {
    if (len > NRF905_MAX_PAYLOAD)
      len = NRF905_MAX_PAYLOAD;

    cselect();
    sleep_ms(10);
    uint8_t buff = NRF905_CMD_W_TX_PAYLOAD;
    spi_write_blocking(spi, &buff, 1);
    spi_write_blocking(spi, data, len);
    sleep_ms(10);
    cdeselect();
  }
}

void nRF905::read(uint8_t *data, uint8_t len) {
  if (len > NRF905_MAX_PAYLOAD)
    len = NRF905_MAX_PAYLOAD;
  cselect();
  sleep_ms(10);
  uint8_t buff = NRF905_CMD_R_RX_PAYLOAD;
  spi_write_blocking(spi, &buff, 1);
  spi_read_blocking(spi, NRF905_CMD_NOP, data, len);
  sleep_ms(10);
  cdeselect();
}

// Must make sure all of the payload has been read, otherwise DR never goes
// low
// uint8_t remaining = NRF905_MAX_PAYLOAD - len;
// while(remaining--)
//	spi.transfer(NRF905_CMD_NOP);
/*
uint32_t nRF905::readUInt32() // TODO
{
        uint32_t data;
        CHIPSELECT()
        {
                spi.transfer(NRF905_CMD_R_RX_PAYLOAD);

                // Get received payload
                for(uint8_t i=0;i<4;i++)
                        ((uint8_t*)&data)[i] = spi.transfer(NRF905_CMD_NOP);
        }
        return data;
}

uint8_t nRF905::readUInt8() // TODO
{
        uint8_t data;
        CHIPSELECT()
        {
                spi.transfer(NRF905_CMD_R_RX_PAYLOAD);

                // Get received payload
                data = spi.transfer(NRF905_CMD_NOP);
        }
        return data;
}
*/
/*
size_t nRF905::write(uint8_t data)
{
        return 1;
}

size_t nRF905::write(const uint8_t *data, size_t quantity)
{
        write(0xE7E7E7E7, data, quantity);
        return quantity;
}

int nRF905::available()
{
        return 1;
}

int nRF905::read()
{
        return 1;
}

int nRF905::peek()
{
        return 1;
}

void nRF905::flush()
{
        // XXX: to be implemented.
}
*/
bool nRF905::TX(nRF905_nextmode_t nextMode, bool collisionAvoid) {
  // TODO check DR is low?
  // check AM for incoming packet?
  // what if already in TX mode? (auto-retransmit or carrier wave)

  nRF905_mode_t currentMode = mode();
  if (currentMode == NRF905_MODE_POWERDOWN) {
    currentMode = NRF905_MODE_STANDBY;
    standbyMode(true);
    powerOn(true);
    if (nextMode != NRF905_NEXTMODE_TX)
      sleep_ms(3); // Delay is needed to the radio has time to power-up and
                   // see the standby/TX pins pulse
  } else if (collisionAvoid && airwayBusy())
    return false;

  // Put into transmit mode
  txMode(true); // PORTB |= _BV(PORTB1);

  // Pulse standby pin to start transmission
  if (currentMode == NRF905_MODE_STANDBY)
    standbyMode(false); // PORTD |= _BV(PORTD7);

  // NOTE: If nextMode is RX or STANDBY and a long running interrupt happens
  // during the delays below then we may end up transmitting a blank carrier
  // wave until the interrupt ends. If nextMode is RX then an unexpected
  // onTxComplete event will also fire and RX mode won't be entered until the
  // interrupt finishes.

  if (nextMode == NRF905_NEXTMODE_RX) {
    // 1.	The datasheets says that the radio can switch straight to RX
    // mode after
    //	a transmission is complete by clearing TX_EN while transmitting,
    // but if
    // the radio was 	in standby mode and TX_EN is cleared within ~700us then
    // the transmission seems to get corrupt.
    // 2.	Going straight to RX also stops DR from pulsing after
    // transmission complete which means the onTxComplete event doesn't work
    if (currentMode == NRF905_MODE_STANDBY) {
      // Use micros() timing here instead of delayMicroseconds() to get better
      // accuracy incase other interrupts happen (which would cause
      // delayMicroseconds() to pause)
      unsigned int start = time_us_64();
      while ((unsigned int)(time_us_64() - start) < 700)
        ;
    } else
      sleep_us(14);
    txMode(false); // PORTB &= ~_BV(PORTB1);
  } else if (nextMode == NRF905_NEXTMODE_STANDBY) {
    sleep_us(14);
    standbyMode(true);
    // txMode(false);
  }
  // else NRF905_NEXTMODE_TX

  return true;
}

void nRF905::RX() {
  txMode(false);
  standbyMode(false);
  powerOn(true);
}

void nRF905::powerDown() { powerOn(false); }

void nRF905::standby() {
  standbyMode(true);
  powerOn(true);
}

nRF905_mode_t nRF905::mode() {
  if (pwr != NRF905_PIN_UNUSED) {
    if (!gpio_get(pwr))
      return NRF905_MODE_POWERDOWN;
  }

  if (trx != NRF905_PIN_UNUSED) {
    if (!gpio_get(trx))
      return NRF905_MODE_STANDBY;
  }

  if (tx != NRF905_PIN_UNUSED) {
    if (gpio_get(tx))
      return NRF905_MODE_TX;
    return NRF905_MODE_RX;
  }

  return NRF905_MODE_ACTIVE;
}
/*
bool nRF905::inStandbyMode()
{
        if(trx != NRF905_PIN_UNUSED)
                return !digitalRead(trx);
        return false;
}

bool nRF905::poweredUp()
{
        if(pwr != NRF905_PIN_UNUSED)
                return digitalRead(pwr);
        return true;
}
*/
void nRF905::getConfigRegisters(uint8_t *regs) {
  cselect();
  sleep_ms(10);
  uint8_t buff = NRF905_CMD_R_CONFIG;
  spi_write_blocking(spi, &buff, 1);
  cdeselect();
  spi_read_blocking(spi, NRF905_CMD_NOP, (uint8_t *)regs,
                    NRF905_REGISTER_COUNT);
  sleep_ms(10);
}

void nRF905::interrupt_dr() {
  // If DR && AM = RX new packet
  // If DR && !AM = TX finished

  if (addressMatched()) {
    validPacket = 1;
    if (onRxComplete != NULL)
      onRxComplete(this);
  } else {
    if (onTxComplete != NULL)
      onTxComplete(this);
  }
}

void nRF905::interrupt_am() {
  // If AM goes HIGH then LOW without DR going HIGH then we got a bad packet

  if (addressMatched()) {
    if (onAddrMatch != NULL)
      onAddrMatch(this);
  } else if (!validPacket) {
    if (onRxInvalid != NULL)
      onRxInvalid(this);
  }
  validPacket = 0;
}

void nRF905::poll() {
  if (!polledMode)
    return;

  static uint8_t lastState;
  static uint8_t addrMatch;

  // TODO read pins if am / dr defined

  uint8_t state =
      readStatus() & ((1 << NRF905_STATUS_DR) | (1 << NRF905_STATUS_AM));

  if (state != lastState) {
    if (state == ((1 << NRF905_STATUS_DR) | (1 << NRF905_STATUS_AM))) {
      addrMatch = 0;
      if (onRxComplete != NULL)
        onRxComplete(this);
    } else if (state == (1 << NRF905_STATUS_DR)) {
      addrMatch = 0;
      if (onTxComplete != NULL)
        onTxComplete(this);
    } else if (state == (1 << NRF905_STATUS_AM)) {
      addrMatch = 1;
      if (onAddrMatch != NULL)
        onAddrMatch(this);
    } else if (state == 0 && addrMatch) {
      addrMatch = 0;
      if (onRxInvalid != NULL)
        onRxInvalid(this);
    }

    lastState = state;
  }
}