#include "logic/battery_diagnosis.h"

battery_diagnosis_t battery_diagnosis(uint32_t millivolts) {
  if (millivolts < BATTERY_LOW_THRESHOLD_MILLIVOLTS) {
    return BATTERY_LOW;
  } else {
    return BATTERY_OK;
  }
}

const char * battery_diagnosis_name(battery_diagnosis_t battery_diagnosis) {
  switch(battery_diagnosis) {
    case BATTERY_OK:
      return "BATTERY_OK";
    case BATTERY_LOW:
      return "BATTERY_LOW";
    default:
      return "UNKNOWN";
  }
}
