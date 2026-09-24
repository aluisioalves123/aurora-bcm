#include <stdint.h>

#include "hal/lm75/driver.h"
#include "board.h"
#include "hal/i2c/driver.h"
#include <libopencm3/stm32/i2c.h>

// guardado entre chamadas: achar o endereco custa ate oito transacoes, e o
// valor so muda quando a placa e reenergizada
static uint8_t found_address = LM75_ADDRESS_UNKNOWN;

uint8_t lm75_address(void) {
  return found_address;
}

static bool read_temperature_at(uint8_t address, uint8_t *bytes) {
  uint8_t temperature_register[1] = { 0x00 };

  return i2c_transfer_with_timeout(I2C1, address,
      temperature_register, sizeof(temperature_register), bytes, 2);
}

temperature_read_t temperature_read(void) {
  temperature_read_t temperature_read = {
    .success = false,
    .value = 0,
  };

  uint8_t temperature_value_bytes[2];
  bool answered = false;

  // caminho normal: o endereco ja conhecido responde
  if (found_address != LM75_ADDRESS_UNKNOWN) {
    answered = read_temperature_at(found_address, temperature_value_bytes);
  }

  // primeira leitura, ou o sensor mudou de endereco: procura na faixa
  if (!answered) {
    found_address = LM75_ADDRESS_UNKNOWN;

    for (uint8_t address = LM75_ADDRESS_FIRST; address <= LM75_ADDRESS_LAST; address++) {
      if (read_temperature_at(address, temperature_value_bytes)) {
        found_address = address;
        answered = true;
        break;
      }
    }
  }

  if (!answered) {
    return temperature_read;
  }

  temperature_read.value = ((uint16_t)temperature_value_bytes[0] << 8)
                         | (uint16_t)temperature_value_bytes[1];
  temperature_read.success = true;

  return temperature_read;
}