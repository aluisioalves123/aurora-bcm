#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define I2C_TIMEOUT 1000

void i2c_setup(void);

bool i2c_recover(uint32_t i2c);

bool i2c_transfer_with_timeout(
  uint32_t i2c,
  uint8_t i2c_address,
  const uint8_t *write_buffer,
  size_t write_length, 
  uint8_t *read_buffer,
  size_t read_length
);

bool i2c_write_with_timeout(uint32_t i2c, int i2c_address, const uint8_t *data, size_t data_length);

bool i2c_read_with_timeout(uint32_t i2c, int i2c_address, uint8_t *response, size_t response_length);

#endif
