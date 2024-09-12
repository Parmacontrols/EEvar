/*
 * EEvar Arduino library
 * author: github.com/AlexIII/EEvar
 * e-mail: endoftheworld@bk.ru
 * license: MIT
 */

#ifndef _EEPROMALLOCATOR_H_
#define _EEPROMALLOCATOR_H_

#include <EEPROM.h>

class EEPROMallocator {
  static uint32_t& addrCnt() {
    static uint32_t cur = 4;  // Reserve first 4 bytes for the allocated bytes
    return cur;
  }

public:
  static void* alloc(const size_t sz) {
#if defined(ESP8266)
    static bool inited = false;
    if(!inited) EEPROM.begin(512);
    inited = true;
#endif
    uint32_t addr = addrCnt();
    const uint32_t start = addr;
    addr += sz;
    if(addr > EEPROM.length()) return NULL;
    addrCnt() = addr;
    if (isFirstStart())
      update_block(&addr, (void*)0, sizeof(addr));
    return (void*)start;
  }

  static uint32_t busy() {
    return addrCnt();
  }

  static uint32_t allocatedBytes() {
    uint32_t bytes;
    read_block(&bytes, (void*)0, sizeof(bytes));
    return bytes;
  }

  static uint32_t free() {
    return addrCnt() >= EEPROM.length()? 0 : EEPROM.length() - addrCnt();
  }

  static bool isFirstStart() {
    return addrCnt() >= allocatedBytes();
  }

  static void resetValuesOnNextBoot() {
    uint32_t zero = 0;
    update_block(&zero, (void*)0, sizeof(zero));
  }

protected:
  EEPROMallocator() {}

#if defined(__AVR__) || defined(ARDUINO_ARCH_AVR)

  static void read_block(void *__dst, const void *__src, size_t __n) { eeprom_read_block(__dst, __src, __n); }
  static void update_block(const void *__src, void *__dst, size_t __n) { eeprom_update_block(__src, __dst, __n); }

#elif defined(__IMXRT1062__)

  static void read_block(void *__dst, const void *__src, size_t __n) { eeprom_read_block(__dst, __src, __n); }
  static void update_block(const void *__src, void *__dst, size_t __n) { eeprom_write_block(__src, __dst, __n); }

#elif defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)

  static void read_block(void *__dst, const void *__src, size_t __n) {
    uint8_t* data = (uint8_t*)__dst;
    uint32_t addr = (uint32_t)__src;
    while(__n--) *data++ = EEPROM[addr++];
  }
  static void update_block(const void *__src, void *__dst, size_t __n) {
    const uint8_t* data = (const uint8_t*)__src;
    uint32_t addr = (uint32_t)__dst;
    while(__n--) EEPROM.update(addr++, *data++);
    EEPROM.commit();
  }

#else

#error "******     EEvar library does not yet support this platform. Supported platforms: AVR, ESP8266     ******"

#endif

};

#endif
