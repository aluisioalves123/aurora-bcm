#include <stdint.h>

#include "hal/lm75.h"
#include "board.h"
#include "hal/i2c.h"
#include <libopencm3/stm32/i2c.h>

temperature_read_t temperature_read(void) {
  temperature_read_t temperature_read = {
    .success = false,
    .value = 0,
  };

  uint8_t temperature_address[1] = { 0x00 };
  uint8_t temperature_value_bytes[2];
  if(i2c_transfer_with_timeout(I2C1, LM75_I2C_ADDRESS, temperature_address, sizeof(temperature_address), temperature_value_bytes, sizeof(temperature_value_bytes))) {
    uint16_t temperature_value = ((uint16_t)temperature_value_bytes[0] << 8) | (uint16_t)temperature_value_bytes[1];
    temperature_read.value = temperature_value;
    temperature_read.success = true;
  }

  return temperature_read;
}