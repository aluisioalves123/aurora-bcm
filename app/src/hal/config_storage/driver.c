#include "driver.h"
#include "libopencm3/stm32/flash.h"

#define STORAGE_ADDRESS 0x08060000

bool config_storage_write(config_t config) {
  flash_unlock();
  uint32_t word_count = sizeof(config)/sizeof(uint32_t);

  flash_erase_sector(7, FLASH_CR_PROGRAM_X32);

  flash_dcache_reset();

  uint32_t *flash = (uint32_t *)STORAGE_ADDRESS;
  for(uint32_t i = 0; i < word_count; i++) {
    if (flash[i] != 0xFFFFFFFF) { flash_lock(); return false; }
  }

  uint32_t *p = (uint32_t *)&config; // agora config é tratado como um array

  for(uint32_t i = 0; i < word_count; i++) {
    flash_program_word(STORAGE_ADDRESS + i*4, p[i]);
  }

  flash_dcache_reset();

  for(uint32_t i = 0; i < word_count; i++) {
    if (flash[i] != p[i]) { flash_lock(); return false; }
  }

  flash_lock();
  
  return true;
}

void config_storage_read(config_t *config) {
  *config = *(const config_t *)STORAGE_ADDRESS;
}