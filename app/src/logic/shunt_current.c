#include <stdint.h>
#include "logic/shunt_current.h"
#include "logic/adc_scale.h"

uint32_t shunt_current(uint32_t adc_value, uint32_t shunt_resistor_ohms) {
    uint32_t shunt_millivolts = adc_value * ADC_REFERENCE_MILLIVOLTS / ADC_FULL_SCALE;
    uint32_t shunt_current_microamperes = shunt_millivolts * 1000 / shunt_resistor_ohms;

    return shunt_current_microamperes;
}