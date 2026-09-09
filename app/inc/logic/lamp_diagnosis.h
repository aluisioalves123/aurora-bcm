#include <stdint.h>
#ifndef INC_LOGIC_LAMP_DIANOSIS_H
#define INC_LOGIC_LAMP_DIANOSIS_H

// o valor da corrente retornada pelo dac com o led desligado foi de 0.003mA, portanto esse valor é apenas uma margem de segurança
#define LAMP_OPEN_THRESHOLD_MICROAMPERES 10

typedef enum {
  LAMP_OK,
  LAMP_OPEN,
  LAMP_UNKNOWN
} lamp_diagnosis_t;

lamp_diagnosis_t lamp_diagnosis(uint32_t shunt_current, uint16_t gpio_status);

const char * lamp_diagnosis_name(lamp_diagnosis_t lamp_diagnosis);

#endif