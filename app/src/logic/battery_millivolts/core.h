#ifndef LOGIC_BATTERY_MILLIVOLTS_H
#define LOGIC_BATTERY_MILLIVOLTS_H

#include <stdint.h>

uint32_t battery_millivolts(uint32_t adc_value, uint32_t top_resistor_ohms, uint32_t bottom_resistor_ohms);

#endif