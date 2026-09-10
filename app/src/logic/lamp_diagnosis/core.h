#include <stdint.h>
#ifndef LOGIC_LAMP_DIAGNOSIS_H
#define LOGIC_LAMP_DIAGNOSIS_H

// Ancorado na corrente nominal, nao no ruido. Medido na bancada: a lampada
// boa e acesa da 477 contagens de ADC, que a cadeia converte em ~1163 uA.
// Com a lampada fora, a leitura passeia entre 0 e 15 uA porque o no fica
// solto, e 10 uA caia dentro desse passeio: o diagnostico alternava sozinho
// entre OK e OPEN.
//
// 500 uA e ~43% do nominal: 33x acima do teto do ruido e 2,3x abaixo da
// lampada boa. Se a carga desta saida mudar, este numero muda junto.
#define LAMP_OPEN_THRESHOLD_MICROAMPERES 500

typedef enum {
  LAMP_OK,
  LAMP_OPEN,
  LAMP_UNKNOWN
} lamp_diagnosis_t;

lamp_diagnosis_t lamp_diagnosis(uint32_t shunt_current, uint16_t gpio_status);

const char * lamp_diagnosis_name(lamp_diagnosis_t lamp_diagnosis);

#endif