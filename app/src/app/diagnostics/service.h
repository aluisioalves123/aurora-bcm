#ifndef INC_APP_DIAGNOSTICS_H
#define INC_APP_DIAGNOSTICS_H

#include "hal/lm75/driver.h"
#include "logic/battery_diagnosis/core.h"
#include "logic/lamp_diagnosis/core.h"
#include "logic/temperature_diagnosis/core.h"

// Le o hardware pelo hal, deixa a decisao com a funcao pura do logic e
// registra o resultado na tabela de falhas.
//
// O registro mora aqui de proposito: hal/ so traduz pino em valor e logic/
// so decide, nenhum dos dois sabe que existe tabela de falha. Quem junta as
// tres coisas e a camada de aplicacao.
lamp_diagnosis_t lamp_diagnosis_update(void);

// Recebe a leitura ja feita em vez de ler de novo: quem chama costuma
// precisar do valor tambem, e cada leitura e uma transacao I2C. Assim o
// numero que aparece no console e exatamente o que gerou o diagnostico.
temperature_diagnosis_t temperature_diagnosis_update(temperature_read_t reading);

// Tambem recebe o valor pronto: quem chama ja converteu para milivolts
// e costuma imprimir o mesmo numero.
battery_diagnosis_t battery_diagnosis_update(uint32_t millivolts);

#endif // INC_APP_DIAGNOSTICS_H
