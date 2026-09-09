#ifndef LOGIC_SHUNT_CURRENT_H
#define LOGIC_SHUNT_CURRENT_H

#include <stdint.h>

uint32_t shunt_current(uint32_t adc_value, uint32_t shunt_resistor_ohms);

#endif