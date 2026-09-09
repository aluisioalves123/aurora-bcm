#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>

#include "hal/adc.h"
#include "board.h"

void adc_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(
        BATTERY_VOLTAGE_PORT,
        GPIO_MODE_ANALOG,
        GPIO_PUPD_NONE,
        BATTERY_VOLTAGE_PIN
    );

    gpio_mode_setup(
        SHUNT_VOLTAGE_PORT,
        GPIO_MODE_ANALOG,
        GPIO_PUPD_NONE,
        SHUNT_VOLTAGE_PIN
    );

    rcc_periph_clock_enable(RCC_ADC1); //clock do conversor
    adc_power_off(ADC_UNIT); // configura com ele desligado

    adc_set_clk_prescale(ADC_CCR_ADCPRE_BY4);
    adc_disable_scan_mode(ADC_UNIT); // um canal por vez
    adc_set_single_conversion_mode(ADC_UNIT); // uma conversão por disparo
    adc_set_sample_time_on_all_channels(ADC_UNIT, ADC_SMPR_SMP_480CYC);   // 480 ciclos

    adc_power_on(ADC_UNIT); //ligar adc

    // espera um pouco apos a primeira leitura para o adc se estabilizar
    for (volatile uint32_t i = 0; i< 10000; i++){
        __asm__("nop");
    }
}

uint32_t adc_read(uint8_t channel) {
    uint8_t sequence[] = { channel };
    adc_set_regular_sequence(ADC_UNIT, 1, sequence);
    adc_start_conversion_regular(ADC_UNIT);
    
    do { /* aguarda a leitura do adc */
        __asm__("nop");
    } while (!adc_eoc(ADC_UNIT));
    
    return adc_read_regular(ADC_UNIT);
}