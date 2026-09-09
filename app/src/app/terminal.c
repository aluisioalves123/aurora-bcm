#include "app/terminal.h"
#include "hal/uart.h"
#include <string.h>
#include <stdio.h>
#include "hal/systick.h"
#include "version.h"
#include "hal/service_light.h"
#include "logic/turn_signal.h"
#include "hal/adc.h"
#include "board.h"
#include "logic/battery_millivolts.h"
#include "logic/shunt_current.h"
#include "logic/lamp_diagnosis.h"

// um comando por funcao. o handle_command la embaixo so escolhe qual chamar,
// e cada uma cuida da propria resposta.

static void command_hello(void) {
  print_serial("hello world\r\n");
}

static void command_help(void) {
  print_serial("/hello = hello world\r\n/status = status da placa e versao");
}

static void command_status(signal_state_t signal_state) {
  char status[256];
  uint32_t uptime_seconds = get_ticks() / 1000;
  uint32_t service_light = service_light_level();
  uint32_t rx_lost = read_rx_lost_bytes();
  uint32_t tx_lost = read_tx_lost_bytes();

  uint32_t battery_value = battery_millivolts(adc_read(BATTERY_ADC_CHANNEL), 1000, 330);
  uint32_t battery_value_int_part = battery_value / 1000;
  uint32_t battery_value_decimal_part = battery_value % 1000;

  uint32_t lamp_shunt_current = shunt_current(adc_read(SHUNT_ADC_CHANNEL), 330);
  uint16_t lamp_gpio_status = gpio_get(LEDS_PORT, TURN_SIGNAL_RIGHT_LED_PIN);

  snprintf(status, sizeof(status),
      "Aurora BCM v%s\r\n"
      "uptime: %lu s\r\n"
      "seta: %s\r\n"
      "lampada direita: %s\r\n"
      "farol: %lu/399\r\n"
      "bateria: %lu.%03lu V\r\n"
      "rx perdidos: %lu\r\n"
      "tx perdidos: %lu\r\n",
      FIRMWARE_VERSION,
      (unsigned long)uptime_seconds,
      signal_state_name(signal_state),
      lamp_diagnosis_name(lamp_diagnosis(lamp_shunt_current, lamp_gpio_status)),
      (unsigned long)service_light,
      (unsigned long)battery_value_int_part,
      (unsigned long)battery_value_decimal_part,
      (unsigned long)rx_lost,
      (unsigned long)tx_lost);

  print_serial(status);
}

static void command_adc_val(void) {
  char adc_val[40];

  snprintf(adc_val, sizeof(adc_val),
      "shunt val: %lu\r\n",
      (unsigned long)adc_read(SHUNT_ADC_CHANNEL));

  print_serial(adc_val);
}

static void command_battery_val(void) {
  char battery_val[40];

  uint32_t adc_value = adc_read(BATTERY_ADC_CHANNEL);
  uint32_t battery_value = battery_millivolts(adc_value, 1000, 330);

  uint32_t battery_value_int_part = battery_value / 1000;
  uint32_t battery_value_decimal_part = battery_value % 1000;

  snprintf(battery_val, sizeof(battery_val),
      "battery val: %lu.%03lu V\r\n",
      (unsigned long)battery_value_int_part,
      (unsigned long)battery_value_decimal_part);

  print_serial(battery_val);
}

static void command_shunt_current(void) {
  char shunt_current_val[40];

  uint32_t adc_value = adc_read(SHUNT_ADC_CHANNEL);
  uint32_t shunt_current_value = shunt_current(adc_value, 330);

  uint32_t shunt_current_value_int_part = shunt_current_value / 1000;
  uint32_t shunt_current_value_decimal_part = shunt_current_value % 1000;

  snprintf(shunt_current_val, sizeof(shunt_current_val),
      "shunt current val: %lu.%03lu mA\r\n",
      (unsigned long)shunt_current_value_int_part,
      (unsigned long)shunt_current_value_decimal_part);

  print_serial(shunt_current_val);
}

static void command_lamp_status(void) {
  uint32_t adc_value = adc_read(SHUNT_ADC_CHANNEL);
  uint32_t shunt_current_value = shunt_current(adc_value, 330);
  uint16_t lamp_gpio_status = gpio_get(LEDS_PORT, TURN_SIGNAL_RIGHT_LED_PIN);

  char lamp_gpio_status_string[40];
  snprintf(lamp_gpio_status_string, sizeof(lamp_gpio_status_string), "%lu\r\n", (unsigned long)lamp_gpio_status);
  print_serial(lamp_gpio_status_string);

  char shunt_current_value_string[40];
  snprintf(shunt_current_value_string, sizeof(shunt_current_value_string), "%lu\r\n", (unsigned long)shunt_current_value);
  print_serial(shunt_current_value_string);

  print_serial(lamp_diagnosis_name(lamp_diagnosis(shunt_current_value, lamp_gpio_status)));
}

static void command_unknown(void) {
  print_serial("comando nao conhecido, consulte a tabela com /help\r\n");
}

void handle_command(const char* prompt, signal_state_t signal_state) {
  if (strcmp(prompt, "/hello") == 0) {
    command_hello();
  } else if (strcmp(prompt, "/help") == 0) {
    command_help();
  } else if (strcmp(prompt, "/status") == 0) {
    command_status(signal_state);
  } else if (strcmp(prompt, "/adc_val") == 0) {
    command_adc_val();
  } else if (strcmp(prompt, "/battery_val") == 0) {
    command_battery_val();
  } else if (strcmp(prompt, "/shunt_current") == 0) {
    command_shunt_current();
  } else if (strcmp(prompt, "/lamp_status") == 0) {
    command_lamp_status();
  } else {
    command_unknown();
  }
}
