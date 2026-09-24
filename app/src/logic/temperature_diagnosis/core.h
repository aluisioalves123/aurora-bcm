#ifndef LOGIC_TEMPERATURE_DIAGNOSIS_H
#define LOGIC_TEMPERATURE_DIAGNOSIS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  TEMPERATURE_OK,
  TEMPERATURE_HIGH,
  TEMPERATURE_UNKNOWN
} temperature_diagnosis_t;

// pura: temperatura em milicelsius + se a leitura veio + o limiar ->
// diagnostico. sem leitura nao ha o que afirmar, e a resposta e UNKNOWN.
//
// o limiar entra por parametro, e nao por #define, porque agora e
// configuravel em tempo de execucao: quem sabe o valor vigente e o config,
// e esta funcao continua sem ler nada de fora.
temperature_diagnosis_t temperature_diagnosis(int32_t millicelsius,
                                              bool sensor_ok,
                                              int32_t high_threshold_millicelsius);

const char * temperature_diagnosis_name(temperature_diagnosis_t temperature_diagnosis);

#endif // LOGIC_TEMPERATURE_DIAGNOSIS_H
