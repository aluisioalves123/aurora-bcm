#include <stdint.h>
#include "logic/battery_millivolts.h"
#include "logic/adc_scale.h"

uint32_t battery_millivolts(uint32_t adc_value, uint32_t resistor_1_value, uint32_t resistor_2_value) {
    uint32_t divisor_node_millivolts = adc_value * ADC_REFERENCE_MILLIVOLTS / ADC_FULL_SCALE;
    uint32_t battery_millivolts =  divisor_node_millivolts * (resistor_1_value + resistor_2_value) / resistor_2_value;

    return battery_millivolts;
}