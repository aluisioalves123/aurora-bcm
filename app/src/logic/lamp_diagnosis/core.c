#include "logic/lamp_diagnosis/core.h"
#include <stdint.h>

lamp_diagnosis_t lamp_diagnosis(uint32_t shunt_current, uint16_t gpio_status) {
  if (shunt_current >= LAMP_OPEN_THRESHOLD_MICROAMPERES && gpio_status != 0) {
    return LAMP_OK;
  } else if (shunt_current <= LAMP_OPEN_THRESHOLD_MICROAMPERES && gpio_status != 0) {
    return LAMP_OPEN;
  } else {
    return LAMP_UNKNOWN;
  }
}

const char * lamp_diagnosis_name(lamp_diagnosis_t lamp_diagnosis) {
  switch(lamp_diagnosis) {
    case LAMP_OK:
      return "LAMP_OK";
    case LAMP_OPEN:
      return "LAMP_OPEN";
    case LAMP_UNKNOWN:
      return "LAMP_UNKNOWN";
    default:
      return "UNKNOWN";
  }
}