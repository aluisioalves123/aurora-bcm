#ifndef INC_HAL_LM75_H
#define INC_HAL_LM75_H

#include <stdint.h>
#include <stdbool.h>

#define LM75_I2C_ADDRESS (0x48)

typedef struct {
  bool success;
  uint16_t value;
} temperature_read_t;

temperature_read_t temperature_read(void);

#endif