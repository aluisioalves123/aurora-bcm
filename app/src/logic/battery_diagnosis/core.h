#ifndef INC_LOGIC_BATTERY_DIAGNOSIS_H
#define INC_LOGIC_BATTERY_DIAGNOSIS_H

#include <stdint.h>

// Bateria de 12 V automotiva: 12,6 V em repouso e carga cheia, 12,0 V perto
// de 25% de carga, 11,8 V praticamente descarregada. Modulos de carroceria
// sinalizam subtensao entre 10,5 e 11,5 V; 11,0 V e o meio dessa faixa.
//
// Cuidado ao mexer neste numero: a partida derruba a tensao para perto de
// 10 V por uma fracao de segundo. Sem tempo de espera antes de registrar, um
// limiar mais alto acusaria falha toda vez que o motorista desse a chave.
#define BATTERY_LOW_THRESHOLD_MILLIVOLTS 11000

typedef enum {
  BATTERY_OK,
  BATTERY_LOW
} battery_diagnosis_t;

// pura: tensao em milivolts -> diagnostico
battery_diagnosis_t battery_diagnosis(uint32_t millivolts);

const char * battery_diagnosis_name(battery_diagnosis_t battery_diagnosis);

#endif // INC_LOGIC_BATTERY_DIAGNOSIS_H
