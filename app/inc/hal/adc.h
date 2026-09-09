#ifndef INC_HAL_ADC_H
#define INC_HAL_ADC_H

void adc_setup(void);

uint32_t adc_read(uint8_t channel);

#endif