#include <stdint.h>
#include "logic/temperature.h"

int32_t temperature(int16_t temperature_raw) {
  int16_t temperature_eighths = temperature_raw >> 5;
  int32_t temperature_millicelsius = temperature_eighths * 125;

  return temperature_millicelsius;
}