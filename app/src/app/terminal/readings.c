// Comandos que respondem o que os sensores estao dizendo agora.
//
// Todos passam pelo app/diagnostics antes de imprimir: o numero que aparece
// no console e exatamente o que alimentou a tabela de falhas, e nao uma
// segunda leitura parecida.

#include "app/terminal/commands.h"

#include <stdio.h>

#include "app/diagnostics/service.h"
#include "board.h"
#include "hal/adc/driver.h"
#include "hal/lm75/driver.h"
#include "hal/service_light/driver.h"
#include "hal/systick/driver.h"
#include "hal/uart/driver.h"
#include "logic/battery_millivolts/core.h"
#include "logic/lamp_diagnosis/core.h"
#include "logic/reset_cause/core.h"
#include "logic/temperature/core.h"
#include "logic/turn_signal/core.h"
#include "version.h"

void command_status(const char *argument, console_t *console) {
  (void)argument;

  char status[320];
  uint32_t uptime_seconds = get_ticks() / 1000;
  uint32_t service_light = service_light_level();
  uint32_t rx_lost = read_rx_lost_bytes();
  uint32_t tx_lost = read_tx_lost_bytes();

  uint32_t battery_value = battery_millivolts(adc_read(BATTERY_ADC_CHANNEL), 1000, 330);

  battery_diagnosis_update(battery_value);
  uint32_t battery_value_int_part = battery_value / 1000;
  uint32_t battery_value_decimal_part = battery_value % 1000;

  lamp_diagnosis_t lamp = lamp_diagnosis_update();

  // a temperatura vira texto antes: sensor mudo nao pode furar a linha toda,
  // o resto do status continua valendo
  char temperature_text[24];
  temperature_read_t reading = temperature_read();

  temperature_diagnosis_update(reading, console->config->temperature_millicelsius_limit);

  if (reading.success) {
    int32_t millicelsius = temperature((int16_t)reading.value);
    const char *sinal = (millicelsius < 0) ? "-" : "";
    int32_t absoluto = (millicelsius < 0) ? -millicelsius : millicelsius;

    snprintf(temperature_text, sizeof(temperature_text), "%s%ld.%03ld C",
        sinal, (long)(absoluto / 1000), (long)(absoluto % 1000));
  } else {
    snprintf(temperature_text, sizeof(temperature_text), "sem resposta");
  }

  snprintf(status, sizeof(status),
      "Aurora BCM v%s\r\n"
      "uptime: %lu s\r\n"
      "ultimo reset: %s\r\n"
      "seta: %s\r\n"
      "lampada direita: %s\r\n"
      "farol: %lu/399\r\n"
      "bateria: %lu.%03lu V\r\n"
      "temperatura: %s\r\n"
      "rx perdidos: %lu\r\n"
      "tx perdidos: %lu\r\n",
      FIRMWARE_VERSION,
      (unsigned long)uptime_seconds,
      reset_cause_text(console->last_reset_cause),
      signal_state_name(console->signal_state),
      lamp_diagnosis_name(lamp),
      (unsigned long)service_light,
      (unsigned long)battery_value_int_part,
      (unsigned long)battery_value_decimal_part,
      temperature_text,
      (unsigned long)rx_lost,
      (unsigned long)tx_lost);

  print_serial(status);
}

void command_battery_val(const char *argument, console_t *console) {
  (void)argument;
  (void)console;

  char battery_val[40];

  uint32_t adc_value = adc_read(BATTERY_ADC_CHANNEL);
  uint32_t battery_value = battery_millivolts(adc_value, 1000, 330);

  battery_diagnosis_update(battery_value);

  uint32_t battery_value_int_part = battery_value / 1000;
  uint32_t battery_value_decimal_part = battery_value % 1000;

  snprintf(battery_val, sizeof(battery_val),
      "battery val: %lu.%03lu V\r\n",
      (unsigned long)battery_value_int_part,
      (unsigned long)battery_value_decimal_part);

  print_serial(battery_val);
}

void command_lamp_status(const char *argument, console_t *console) {
  (void)argument;
  (void)console;

  print_serial(lamp_diagnosis_name(lamp_diagnosis_update()));
  print_serial("\r\n");
}

void command_temperature(const char *argument, console_t *console) {
  (void)argument;

  char temperature_string[48];
  temperature_read_t reading = temperature_read();

  temperature_diagnosis_update(reading, console->config->temperature_millicelsius_limit);

  if (!reading.success) {
    print_serial("temperature: sensor nao respondeu\r\n");
    return;
  }

  int32_t millicelsius = temperature((int16_t)reading.value);

  // o sinal sai separado: com valor negativo, o resto da divisao tambem vem
  // negativo, e "%ld.%03ld" imprimiria algo como -5.-250
  const char *sinal = (millicelsius < 0) ? "-" : "";
  int32_t absoluto = (millicelsius < 0) ? -millicelsius : millicelsius;

  snprintf(temperature_string, sizeof(temperature_string),
      "temperature: %s%ld.%03ld C\r\n",
      sinal,
      (long)(absoluto / 1000),
      (long)(absoluto % 1000));

  print_serial(temperature_string);
}
