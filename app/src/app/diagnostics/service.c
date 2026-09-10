#include "app/diagnostics/service.h"

#include "board.h"
#include "hal/adc/driver.h"
#include "hal/systick/driver.h"
#include "logic/fault_table/core.h"
#include "logic/shunt_current/core.h"
#include "logic/temperature/core.h"

lamp_diagnosis_t lamp_diagnosis_update(void) {
  uint32_t lamp_shunt_current = shunt_current(adc_read(SHUNT_ADC_CHANNEL), 330);
  uint16_t lamp_gpio_status = gpio_get(LEDS_PORT, TURN_SIGNAL_RIGHT_LED_PIN);

  lamp_diagnosis_t diagnosis = lamp_diagnosis(lamp_shunt_current, lamp_gpio_status);

  if (diagnosis == LAMP_OPEN) {
    fault_report(FAULT_LAMP_OPEN, get_ticks());
  } else if (diagnosis == LAMP_OK) {
    fault_clear(FAULT_LAMP_OPEN);
  }

  // LAMP_UNKNOWN nao mexe na tabela: com a saida desligada nao passa
  // corrente de qualquer jeito, entao nao da para afirmar nada sobre o
  // filamento. nem registrar nem limpar seria honesto.

  return diagnosis;
}

temperature_diagnosis_t temperature_diagnosis_update(temperature_read_t reading) {
  int32_t millicelsius = reading.success ? temperature((int16_t)reading.value) : 0;

  temperature_diagnosis_t diagnosis = temperature_diagnosis(millicelsius, reading.success);

  // o sensor calado e uma falha por si so, independente do valor: e o que
  // diferencia "esta frio" de "nao sei dizer"
  if (reading.success) {
    fault_clear(FAULT_SENSOR_NOT_RESPONDING);
  } else {
    fault_report(FAULT_SENSOR_NOT_RESPONDING, get_ticks());
  }

  if (diagnosis == TEMPERATURE_HIGH) {
    fault_report(FAULT_TEMPERATURE_HIGH, get_ticks());
  } else if (diagnosis == TEMPERATURE_OK) {
    fault_clear(FAULT_TEMPERATURE_HIGH);
  }

  // TEMPERATURE_UNKNOWN nao mexe na tabela: sensor mudo nao e prova de que
  // a temperatura esta boa nem de que esta alta.

  return diagnosis;
}

battery_diagnosis_t battery_diagnosis_update(uint32_t millivolts) {
  battery_diagnosis_t diagnosis = battery_diagnosis(millivolts);

  if (diagnosis == BATTERY_LOW) {
    fault_report(FAULT_BATTERY_LOW, get_ticks());
  } else {
    fault_clear(FAULT_BATTERY_LOW);
  }

  return diagnosis;
}
