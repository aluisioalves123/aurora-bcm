#ifndef LOGIC_CONFIG_H
#define LOGIC_CONFIG_H

#define CONFIG_SIGNATURE 0xAABBCCDD

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t magic; // assinatura da configuração
  int32_t temperature_millicelsius_limit;
} config_t;

static const config_t config_default = {
  .magic      = CONFIG_SIGNATURE,

  // Valor de bancada, escolhido para dar para provocar a falha com o dedo no
  // sensor. Nao e um limite de produto: num modulo de carroceria isso seria
  // dezenas de graus acima, ancorado no que a eletronica aguenta.
  .temperature_millicelsius_limit = 24000
};

bool config_validate(config_t raw_config);

#endif