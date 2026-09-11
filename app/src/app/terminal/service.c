#include "app/terminal/service.h"
#include "hal/uart/driver.h"
#include <string.h>
#include <stdio.h>
#include "hal/systick/driver.h"
#include "version.h"
#include "hal/service_light/driver.h"
#include "logic/turn_signal/core.h"
#include "hal/adc/driver.h"
#include "board.h"
#include "logic/battery_millivolts/core.h"
#include "logic/shunt_current/core.h"
#include "logic/lamp_diagnosis/core.h"
#include "hal/lm75/driver.h"
#include "logic/temperature/core.h"
#include "app/diagnostics/service.h"
#include "logic/fault_table/core.h"
#include "logic/fault_table/core.h"
#include "logic/reset_cause/core.h"
#include "hal/sd_card/driver.h"
#include "hal/reset_cause/driver.h"

// um comando por funcao. o handle_command la embaixo so escolhe qual chamar,
// e cada uma cuida da propria resposta.

static void command_hello(void) {
  print_serial("hello world\r\n");
}

static void command_help(void) {
  print_serial(
      "comandos disponiveis:\r\n"
      "  /hello           hello world\r\n"
      "  /help            esta lista\r\n"
      "  /status          estado geral da placa\r\n"
      "  /adc_val         leitura crua do canal do shunt\r\n"
      "  /battery_val     tensao da bateria, em volts\r\n"
      "  /shunt_current   corrente pelo shunt, em miliamperes\r\n"
      "  /lamp_status     diagnostico da lampada da seta direita\r\n"
      "  /temperature_raw valor cru do LM75, decimal e hexadecimal\r\n"
      "  /temperature     temperatura em graus Celsius\r\n"
      "  /fault_list      tipos de falha que podem ser consultados\r\n"
      "  /fault <tipo>    estado de uma falha: ativa, ocorrencias e quando\r\n"
      "  /sd_init         acorda o cartao SD, manda CMD0, CMD8 e inicializa\r\n"
      "  /sd_addressing   se o cartao endereca por bloco ou por byte\r\n");
}

static void command_status(signal_state_t signal_state, reset_cause_t last_reset_cause) {
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

  temperature_diagnosis_update(reading);

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
      reset_cause_text(last_reset_cause),
      signal_state_name(signal_state),
      lamp_diagnosis_name(lamp),
      (unsigned long)service_light,
      (unsigned long)battery_value_int_part,
      (unsigned long)battery_value_decimal_part,
      temperature_text,
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

  battery_diagnosis_update(battery_value);

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
  print_serial(lamp_diagnosis_name(lamp_diagnosis_update()));
  print_serial("\r\n");
}

static void command_temperature_raw(void) {
  char temperature_val[48];
  temperature_read_t reading = temperature_read();

  temperature_diagnosis_update(reading);

  if (!reading.success) {
    print_serial("temperature raw: sensor nao respondeu\r\n");
    return;
  }

  // cru, em decimal e em hexadecimal: o hex deixa ver os dois bytes
  // separados, que e o que interessa para conferir o deslocamento
  snprintf(temperature_val, sizeof(temperature_val),
      "temperature raw: %lu (0x%04lX)\r\n",
      (unsigned long)reading.value,
      (unsigned long)reading.value);

  print_serial(temperature_val);
}

static void command_temperature(void) {
  char temperature_string[48];
  temperature_read_t reading = temperature_read();

  temperature_diagnosis_update(reading);

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

static void command_fault_list(void) {
  char linha[64];
  int i;

  print_serial("tipos de falha que podem ser consultados:\r\n");

  for (i = 0; i < FAULT_COUNT; i++) {
    snprintf(linha, sizeof(linha), "  %s\r\n", fault_code_name((fault_code_t)i));
    print_serial(linha);
  }

  print_serial("use /fault <tipo> para ver o estado de um deles\r\n");
}

static void command_fault(const char *tipo) {
  char linha[128];
  int i;

  for (i = 0; i < FAULT_COUNT; i++) {
    if (strcmp(tipo, fault_code_name((fault_code_t)i)) == 0) {
      fault_entry_t entrada = fault_get((fault_code_t)i);

      snprintf(linha, sizeof(linha),
          "%s\r\n"
          "  ativa: %s\r\n"
          "  ocorrencias: %lu\r\n"
          "  vista pela ultima vez em: %lu ms\r\n",
          fault_code_name((fault_code_t)i),
          entrada.active ? "sim" : "nao",
          (unsigned long)entrada.count,
          (unsigned long)entrada.last_seen_at);

      print_serial(linha);
      return;
    }
  }

  print_serial("tipo de falha desconhecido, veja a lista com /fault_list\r\n");
}

static void command_sd_init(void) {
  char linha[64];

  // os 80 pulsos de acordar primeiro, o CMD0 depois: o cartao so escuta
  // comando depois de ver clock suficiente com o CS alto
  wake_up();

  snprintf(linha, sizeof(linha), "sd cmd0: 0x%02X\r\n", (unsigned)spi_mode_config());
  print_serial(linha);

  sd_response_t cmd8 = check_interface_condition();

  snprintf(linha, sizeof(linha),
      "sd cmd8: r1=0x%02X payload=0x%02X 0x%02X 0x%02X 0x%02X\r\n",
      (unsigned)cmd8.r1,
      (unsigned)cmd8.payload[0],
      (unsigned)cmd8.payload[1],
      (unsigned)cmd8.payload[2],
      (unsigned)cmd8.payload[3]);
  print_serial(linha);

  snprintf(linha, sizeof(linha), "sd init: %s\r\n",
      initialize_card() ? "ok" : "falhou");
  print_serial(linha);
}

static void command_sd_addressing(void) {
  char linha[48];

  snprintf(linha, sizeof(linha), "sd enderecamento: %s\r\n",
      card_uses_block_addressing() ? "bloco" : "byte");
  print_serial(linha);
}

static void command_unknown(void) {
  print_serial("comando nao conhecido, consulte a tabela com /help\r\n");
}

void handle_command(const char* prompt, signal_state_t signal_state, reset_cause_t last_reset_cause) {
  if (strcmp(prompt, "/hello") == 0) {
    command_hello();
  } else if (strcmp(prompt, "/help") == 0) {
    command_help();
  } else if (strcmp(prompt, "/status") == 0) {
    command_status(signal_state, last_reset_cause);
  } else if (strcmp(prompt, "/adc_val") == 0) {
    command_adc_val();
  } else if (strcmp(prompt, "/battery_val") == 0) {
    command_battery_val();
  } else if (strcmp(prompt, "/shunt_current") == 0) {
    command_shunt_current();
  } else if (strcmp(prompt, "/lamp_status") == 0) {
    command_lamp_status();
  } else if (strcmp(prompt, "/temperature_raw") == 0) {
    command_temperature_raw();
  } else if (strcmp(prompt, "/temperature") == 0) {
    command_temperature();
  } else if (strcmp(prompt, "/fault_list") == 0) {
    command_fault_list();
  } else if (strncmp(prompt, "/fault ", 7) == 0) {
    command_fault(prompt + 7);
  } else if (strcmp(prompt, "/sd_init") == 0) {
    command_sd_init();
  } else if (strcmp(prompt, "/sd_addressing") == 0) {
    command_sd_addressing();
  } else {
    command_unknown();
  }
}
