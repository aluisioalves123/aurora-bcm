#ifndef INC_LOGIC_TEMPERATURE_DIAGNOSIS_H
#define INC_LOGIC_TEMPERATURE_DIAGNOSIS_H

#include <stdint.h>
#include <stdbool.h>

// Valor de bancada, escolhido para dar para provocar a falha com o dedo no
// sensor. Nao e um limite de produto: num modulo de carroceria isso seria
// dezenas de graus acima, ancorado no que a eletronica aguenta.
#define TEMPERATURE_HIGH_THRESHOLD_MILLICELSIUS 24000

typedef enum {
  TEMPERATURE_OK,
  TEMPERATURE_HIGH,
  TEMPERATURE_UNKNOWN
} temperature_diagnosis_t;

// pura: temperatura em milicelsius + se a leitura veio -> diagnostico.
// sem leitura nao ha o que afirmar, e a resposta e UNKNOWN.
temperature_diagnosis_t temperature_diagnosis(int32_t millicelsius, bool sensor_ok);

const char * temperature_diagnosis_name(temperature_diagnosis_t temperature_diagnosis);

#endif // INC_LOGIC_TEMPERATURE_DIAGNOSIS_H
