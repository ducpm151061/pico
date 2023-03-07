#include "at45db.h"

// Dataflash commands
#define FlashPageRead 0xD2 // Main memory page read
#define StatusReg 0xD7     // Status register
#define ReadMfgID 0x9F     // Read Manufacturer and Device ID
#define PageErase 0x81     // Page erase
#define ReadSecReg 0x77    // Read Security Register

#define FlashToBuf1Transfer 0x53 // Main memory page to buffer 1 transfer
#define Buf1Read 0xD4            // Buffer 1 read
#define Buf1ToFlashWE                                                          \
  0x83 // Buffer 1 to main memory page program with built-in erase
#define Buf1Write 0x84 // Buffer 1 write

#define FlashToBuf2Transfer 0x55 // Main memory page to buffer 2 transfer
#define Buf2Read 0xD6            // Buffer 2 read
#define Buf2ToFlashWE                                                          \
  0x86 // Buffer 2 to main memory page program with built-in erase
#define Buf2Write 0x87 // Buffer 2 write

void At45db::init(uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin,
                  uint8_t ssPin) {
  spi_init(spi_default, 1000 * 1000);
  gpio_set_function(misoPin, GPIO_FUNC_SPI);
  gpio_set_function(sckPin, GPIO_FUNC_SPI);
  gpio_set_function(mosiPin, GPIO_FUNC_SPI);
  // Make the SPI pins available to picotool
  bi_decl(bi_3pins_with_func(misoPin, mosiPin, sckPin, GPIO_FUNC_SPI));

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_init(ssPin);
  gpio_put(ssPin, 1);
  gpio_set_dir(ssPin, GPIO_OUT);
  // Make the CS pin available to picotool
  bi_decl(bi_1pin_with_name(ssPin, "SPI CS"));
#if DF_VARIANT == DF_AT45DB081D
  _pageAddrShift = 1;
#elif DF_VARIANT == DF_AT45DB161D
  _pageAddrShift = 1;
#elif DF_VARIANT == DF_AT45DB041D
  _pageAddrShift = 1;
#endif
}

static inline void cs_select(uint cs_pin) {
  asm volatile("nop \n nop \n nop");
  gpio_put(cs_pin, 0);
  asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
  asm volatile("nop \n nop \n nop");
  gpio_put(cs_pin, 1);
  asm volatile("nop \n nop \n nop");
}

void At45db::read(uint8_t reg, uint8_t *buf, uint16_t len) {
  cs_select();
  spi_write_blocking(SPI_PORT, &reg, 1);
  sleep_ms(10);
  spi_read_blocking(SPI_PORT, 0, buf, len);
  cs_deselect();
  sleep_ms(10);
}

void At45db::write(uint8_t reg, uint8_t *buf, uint16_t len) {
  cs_select();

  uint8_t buf[len];

  spi_write_blocking(spi_default, buf, 2);
  cs_deselect();
}
uint8_t At45db::readStatus() {
  unsigned char result;

  activate();
  result = transmit(StatusReg);
  result = transmit(0x00);
  deactivate();

  return result;
}

// Monitor the status register, wait until busy-flag is high
void At45db::waitTillReady() {
  while (!(readStatus() & 0x80)) {
    // WDT reset maybe??
  }
}

void At45db::readID(uint8_t *data) {
  activate();
  transmit(ReadMfgID);
  data[0] = transmit(0x00);
  data[1] = transmit(0x00);
  data[2] = transmit(0x00);
  data[3] = transmit(0x00);
  deactivate();
}

// Reads a number of bytes from one of the Dataflash security register
void At45db::readSecurityReg(uint8_t *data, uint16_t size) {
  activate();
  transmit(ReadSecReg);
  transmit(0x00);
  transmit(0x00);
  transmit(0x00);
  for (uint16_t i = 0; i < size; i++) {
    *data++ = transmit(0x00);
  }
  deactivate();
}

// Transfers a page from flash to Dataflash SRAM buffer
void At45db::readPageToBuf1(uint16_t pageAddr) {
  activate();
  transmit(FlashToBuf1Transfer);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t At45db::readByteBuf1(uint16_t addr) {
  unsigned char data = 0;

  activate();
  transmit(Buf1Read);
  transmit(0x00); // don't care
  transmit((uint8_t)(addr >> 8));
  transmit((uint8_t)(addr));
  transmit(0x00);        // don't care
  data = transmit(0x00); // read byte
  deactivate();

  return data;
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void At45db::readStrBuf1(uint16_t addr, uint8_t *data, uint16_t size) {
  activate();
  transmit(Buf1Read);
  transmit(0x00); // don't care
  transmit((uint8_t)(addr >> 8));
  transmit((uint8_t)(addr));
  transmit(0x00); // don't care
  for (uint16_t i = 0; i < size; i++) {
    *data++ = transmit(0x00);
  }
  deactivate();
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void At45db::writeByteBuf1(uint16_t addr, uint8_t data) {
  activate();
  transmit(Buf1Write);
  transmit(0x00); // don't care
  transmit((uint8_t)(addr >> 8));
  transmit((uint8_t)(addr));
  transmit(data); // write data byte
  deactivate();
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void At45db::writeStrBuf1(uint16_t addr, uint8_t *data, uint16_t size) {
  activate();
  transmit(Buf1Write);
  transmit(0x00); // don't care
  transmit((uint8_t)(addr >> 8));
  transmit((uint8_t)(addr));
  for (uint16_t i = 0; i < size; i++) {
    transmit(*data++);
  }
  deactivate();
}

// Transfers Dataflash SRAM buffer 1 to flash page
void At45db::writeBuf1ToPage(uint16_t pageAddr) {
  activate();
  transmit(Buf1ToFlashWE);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

void At45db::pageErase(uint16_t pageAddr) {
  cs_select();
  transmit(PageErase);
  setPageAddr(pageAddr);
  cs_deselect();
  waitTillReady();
}

void At45db::chipErase() {
  cs_select() transmit(0xC7);
  transmit(0x94);
  transmit(0x80);
  transmit(0x9A);
  cs_deselect();
  waitTillReady();
}

void At45db::setPageAddr(unsigned int pageAddr) {
  transmit(getPageAddrByte0(pageAddr));
  transmit(getPageAddrByte1(pageAddr));
  transmit(getPageAddrByte2(pageAddr));
}

/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *    followed by three address bytes consist of three don’t care bits,
 *    12 page address bits (PA11 - PA0) that specify the page in the main
 *    memory to be written and nine don’t care bits."
 */
/*
 * From the AT45DB161B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */
/*
 * From the AT45DB041D documentation
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *   followed by three address bytes consist of four don’t care bits, 11 page
 *   address bits (PA10 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 */
uint8_t At45db::getPageAddrByte0(uint16_t pageAddr) {
  // More correct would be to use a 24 bits number
  // shift to the left by number of bits. But the uint16_t can be considered
  // as if it was already shifted by 8.
  return (pageAddr << (DF_PAGE_BITS - 8)) >> 8;
}
uint8_t At45db::getPageAddrByte1(uint16_t page) {
  return page << (DF_PAGE_BITS - 8);
}
uint8_t At45db::getPageAddrByte2(uint16_t page) { return 0; }

// Use a single common instance
At45db dflash;