#include <stdint.h>
#ifndef LOGIC_LAMP_DIAGNOSIS_H
#define LOGIC_LAMP_DIAGNOSIS_H

#define LAMP_OPEN_THRESHOLD_MICROAMPERES 500

typedef enum {
  LAMP_OK,
  LAMP_OPEN,
  LAMP_UNKNOWN
} lamp_diagnosis_t;

lamp_diagnosis_t lamp_diagnosis(uint32_t shunt_current, uint16_t gpio_status);

const char * lamp_diagnosis_name(lamp_diagnosis_t lamp_diagnosis);

#endif