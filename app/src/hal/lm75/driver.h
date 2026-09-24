#ifndef HAL_LM75_H
#define HAL_LM75_H

#include <stdint.h>
#include <stdbool.h>

// Os pinos A0, A1 e A2 do chip definem os tres bits de baixo do endereco, e
// nesta plaquinha eles nao estao amarrados: cada energizacao assenta num
// valor diferente. Medido na bancada, o mesmo modulo apareceu em 0x4A, 0x4B
// e 0x4F em boots seguidos, sempre firme dentro de cada boot.
//
// Por isso o endereco e procurado em vez de fixado. A faixa e a familia
// inteira do LM75: 0x48 mais os tres bits.
#define LM75_ADDRESS_FIRST   (0x48)
#define LM75_ADDRESS_LAST    (0x4F)
#define LM75_ADDRESS_UNKNOWN (0xFF)

// devolve o endereco em que o sensor respondeu por ultimo, ou
// LM75_ADDRESS_UNKNOWN se ainda nao foi encontrado
uint8_t lm75_address(void);

typedef struct {
  bool success;
  uint16_t value;
} temperature_read_t;

temperature_read_t temperature_read(void);

#endif