#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define DF_AT45DB081D 1
#define DF_AT45DB161D 2
#define DF_AT45DB041D 3
#define DF_AT45DB321D 4

#ifndef DF_VARIANT
#define DF_VARIANT DF_AT45DB161D
#endif

#if DF_VARIANT == DF_AT45DB081D
// configuration for the Atmel AT45DB081D device, Sodaq v2 has AT45DB081D, see
doc3596.pdf, 4096 pages of 256 / 264 bytes #define DF_PAGE_ADDR_BITS 12
#define DF_PAGE_SIZE 264
#define DF_PAGE_BITS 9
/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *    followed by three address bytes consist of three don’t care bits,
 *    12 page address bits (PA11 - PA0) that specify the page in the main
 *    memory to be written and nine don’t care bits."
 */
#define DF_AT45DB081D 1
#define DF_AT45DB161D 2
#define DF_AT45DB041D 3
#define DF_AT45DB321D 4

#define DF_VARIANT DF_AT45DB041D

#elif DF_VARIANT == DF_AT45DB161D
// configuration for the Atmel AT45DB161D device
#define DF_PAGE_ADDR_BITS 12
#define DF_PAGE_SIZE 528
#define DF_PAGE_BITS 10
/*
 * From the AT45DB161B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB041D
// configuration for the Atmel AT45DB041D device
#define DF_PAGE_ADDR_BITS 11
#define DF_PAGE_SIZE 264
#define DF_PAGE_BITS 9
/*
 * From AT45DB041D documentation
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *   followed by three address bytes consist of four don’t care bits, 11 page
 *   address bits (PA10 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 */
#elif DF_VARIANT == DF_AT45DB321D
// configuration for the Atmel AT45DB321D device
#define DF_PAGE_ADDR_BITS 13
#define DF_PAGE_SIZE 528
#define DF_PAGE_BITS 10
/*
 * From the AT45DB321B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 1 don’t care bits, 13 page
 *    address bits (PA12 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */

#else
#error "Unknown DF_VARIANT"
#endif
#define DF_NR_PAGES (1 << DF_PAGE_ADDR_BITS)

                 class At45db {
public:
  void init(uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin, uint8_t ssPin)
      __attribute__((deprecated("Use: void init(uint8_t csPin=SS)")));
  void readID(uint8_t *data);
  void readSecurityReg(uint8_t *data, uint16_t size);

  uint8_t readByteBuf1(uint16_t pageAddr);
  void readStrBuf1(uint16_t addr, uint8_t *data, uint16_t size);
  void writeByteBuf1(uint16_t addr, uint8_t data);
  void writeStrBuf1(uint16_t addr, uint8_t *data, uint16_t size);

  void writeBuf1ToPage(uint16_t pageAddr);
  void readPageToBuf1(uint16_t PageAdr);

  void pageErase(uint16_t pageAddr);
  void chipErase();

private:
  uint8_t readStatus();
  void cs_select();
  void cs_deselect();
  void waitTillReady();
  void setPageAddr(unsigned int PageAdr);
  uint8_t getPageAddrByte0(uint16_t pageAddr);
  uint8_t getPageAddrByte1(uint16_t pageAddr);
  uint8_t getPageAddrByte2(uint16_t pageAddr);

  uint16_t _pageAddrShift;
  uint8_t _ss;
  spi_inst_t *_spi;
};

extern At45db at45db;