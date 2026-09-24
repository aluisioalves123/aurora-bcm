#include "logic/temperature_diagnosis/core.h"

temperature_diagnosis_t temperature_diagnosis(int32_t millicelsius,
                                              bool sensor_ok,
                                              int32_t high_threshold_millicelsius) {
  if (!sensor_ok) {
    return TEMPERATURE_UNKNOWN;
  } else if (millicelsius > high_threshold_millicelsius) {
    return TEMPERATURE_HIGH;
  } else {
    return TEMPERATURE_OK;
  }
}

const char * temperature_diagnosis_name(temperature_diagnosis_t temperature_diagnosis) {
  switch(temperature_diagnosis) {
    case TEMPERATURE_OK:
      return "TEMPERATURE_OK";
    case TEMPERATURE_HIGH:
      return "TEMPERATURE_HIGH";
    case TEMPERATURE_UNKNOWN:
      return "TEMPERATURE_UNKNOWN";
    default:
      return "UNKNOWN";
  }
}
