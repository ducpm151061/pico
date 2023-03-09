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

#define SCK_PIN 2
#define MOSI_PIN 3
#define MISO_PIN 4
#define CS_PIN 5

void At45db::init() {
  _ss = CS_PIN;
  _spi = spi0;
  spi_init(_spi, 10 * 1000 * 1000);
  gpio_set_function(MISO_PIN, GPIO_FUNC_SPI);
  gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
  // Make the SPI pins available to picotool
  bi_decl(bi_3pins_with_func(MISO_PIN, MOSI_PIN, SCK_PIN, GPIO_FUNC_SPI));

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_init(_ss);
  gpio_put(_ss, 1);
  gpio_set_dir(_ss, GPIO_OUT);
  // Make the CS pin available to picotool
  bi_decl(bi_1pin_with_name(_ss, "SPI CS"));
#if DF_VARIANT == DF_AT45DB081D
  _pageAddrShift = 1;
#elif DF_VARIANT == DF_AT45DB161D
  _pageAddrShift = 1;
#elif DF_VARIANT == DF_AT45DB041D
  _pageAddrShift = 1;
#endif
}

void At45db::cs_select() {
  asm volatile("nop \n nop \n nop");
  gpio_put(_ss, 0);
  asm volatile("nop \n nop \n nop");
}

void At45db::cs_deselect() {
  asm volatile("nop \n nop \n nop");
  gpio_put(_ss, 1);
  asm volatile("nop \n nop \n nop");
}

uint8_t At45db::readStatus() {
  uint8_t reg = StatusReg;
  uint8_t result;
  cs_select();
  spi_write_blocking(_spi, &reg, 1);
  sleep_ms(10);
  spi_read_blocking(_spi, 0x00, &result, 1);
  cs_deselect();
  sleep_ms(10);
  return result;
}

// Monitor the status register, wait until busy-flag is high
void At45db::waitTillReady() {
  while (!(readStatus() & 0x80)) {
    // WDT reset maybe??
  }
}

void At45db::readID(uint8_t *data) {
  uint8_t reg = ReadMfgID;
  cs_select();
  spi_write_blocking(_spi, &reg, 1);
  sleep_ms(10);
  spi_read_blocking(_spi, 0x00, data, 4);
  cs_deselect();
  sleep_ms(10);
}

// Reads a number of bytes from one of the Dataflash security register
void At45db::readSecurityReg(uint8_t *data, uint16_t size) {
  uint8_t reg = ReadSecReg;
  cs_select();
  spi_write_blocking(_spi, &reg, 1);
  sleep_ms(10);
  spi_read_blocking(_spi, 0x00, data, size);
  cs_deselect();
  sleep_ms(10);
}

// Transfers a page from flash to Dataflash SRAM buffer
void At45db::readPageToBuf1(uint16_t pageAddr) {
  uint8_t reg = FlashToBuf1Transfer;
  cs_select();
  spi_write_blocking(_spi, &reg, 1);
  sleep_ms(10);
  setPageAddr(pageAddr);
  cs_deselect();
  waitTillReady();
  sleep_ms(10);
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t At45db::readByteBuf1(uint16_t addr) {
  uint8_t reg[] = {Buf1Read, 0x00, (uint8_t)(addr >> 8), (uint8_t)(addr), 0x00};
  uint8_t result;
  cs_select();
  spi_write_blocking(_spi, reg, 5);
  sleep_ms(10);
  spi_read_blocking(_spi, 0x00, &result, 1);
  cs_deselect();
  sleep_ms(10);
  return result;
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void At45db::readStrBuf1(uint16_t addr, uint8_t *data, uint16_t size) {
  uint8_t reg[] = {Buf1Read, 0x00, (uint8_t)(addr >> 8), (uint8_t)(addr), 0x00};
  cs_select();
  spi_write_blocking(_spi, reg, 5);
  sleep_ms(10);
  spi_read_blocking(_spi, 0x00, data, size);
  cs_deselect();
  sleep_ms(10);
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void At45db::writeByteBuf1(uint16_t addr, uint8_t data) {
  uint8_t reg[] = {Buf1Write, 0x00, (uint8_t)(addr >> 8), (uint8_t)(addr),
                   data};
  cs_select();
  spi_write_blocking(_spi, reg, 5);
  sleep_ms(10);
  cs_deselect();
  sleep_ms(10);
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void At45db::writeStrBuf1(uint16_t addr, uint8_t *data, uint16_t size) {
  uint8_t reg[] = {Buf1Write, 0x00, (uint8_t)(addr >> 8), (uint8_t)(addr)};
  cs_select();
  spi_write_blocking(_spi, reg, 4);
  spi_write_blocking(_spi, data, size);
  cs_deselect();
}

// Transfers Dataflash SRAM buffer 1 to flash page
void At45db::writeBuf1ToPage(uint16_t pageAddr) {
  uint8_t buf = Buf1ToFlashWE;
  cs_select();
  spi_write_blocking(_spi, &buf, 1);
  sleep_ms(10);
  setPageAddr(pageAddr);
  cs_deselect();
  waitTillReady();
  sleep_ms(10);
}

void At45db::pageErase(uint16_t pageAddr) {
  uint8_t buf = PageErase;
  cs_select();
  spi_write_blocking(_spi, &buf, 1);
  setPageAddr(pageAddr);
  cs_deselect();
  waitTillReady();
}

void At45db::chipErase() {
  uint8_t buf[] = {0xC7, 0x94, 0x80, 0x9A};
  cs_select();
  spi_write_blocking(_spi, buf, 4);
  sleep_ms(10);
  cs_deselect();
  waitTillReady();
  sleep_ms(10);
}

void At45db::setPageAddr(unsigned int pageAddr) {
  uint8_t reg[] = {getPageAddrByte0(pageAddr), getPageAddrByte1(pageAddr),
                   getPageAddrByte2(pageAddr)};

  spi_write_blocking(_spi, reg, 3);
  sleep_ms(10);
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
