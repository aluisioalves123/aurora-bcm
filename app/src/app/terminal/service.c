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
#include "app/config/service.h"
#include "ff.h"
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
      "  /log             acrescenta uma linha em log.txt no cartao\r\n"
      "  /log_read        mostra o conteudo de log.txt\r\n"
      "  /config <n> <v>  grava uma configuracao na flash (ex: /config temperature_limit 24.5)\r\n");
}

static void command_status(signal_state_t signal_state, reset_cause_t last_reset_cause,
                           int32_t temperature_limit_millicelsius) {
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

  temperature_diagnosis_update(reading, temperature_limit_millicelsius);

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

static void command_temperature_raw(int32_t temperature_limit_millicelsius) {
  char temperature_val[48];
  temperature_read_t reading = temperature_read();

  temperature_diagnosis_update(reading, temperature_limit_millicelsius);

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

static void command_temperature(int32_t temperature_limit_millicelsius) {
  char temperature_string[48];
  temperature_read_t reading = temperature_read();

  temperature_diagnosis_update(reading, temperature_limit_millicelsius);

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

// O FIL e static so por causa da pilha: ele carrega um setor inteiro dentro,
// sao ~560 bytes. Ao contrario do FATFS, nao precisa sobreviver a funcao — o
// f_close encerra o uso dele aqui mesmo.
static FIL log_file;

static void command_log(void) {
  char linha[64];
  const char texto[] = "Aurora BCM - teste de escrita\r\n";
  UINT escritos = 0;

  // FA_OPEN_APPEND cria o arquivo se nao existir e posiciona no fim se existir,
  // que e o comportamento de log
  FRESULT abertura = f_open(&log_file, "log.txt", FA_WRITE | FA_OPEN_APPEND);

  snprintf(linha, sizeof(linha), "f_open: %s (codigo %d)\r\n",
      abertura == FR_OK ? "ok" : "falhou", (int)abertura);
  print_serial(linha);

  if (abertura != FR_OK) {
    return;
  }

  // sizeof - 1 para nao gravar o terminador: ele fecha a string na memoria,
  // nao faz parte do texto
  FRESULT escrita = f_write(&log_file, texto, sizeof(texto) - 1, &escritos);

  snprintf(linha, sizeof(linha), "f_write: %s (codigo %d, %u bytes)\r\n",
      escrita == FR_OK ? "ok" : "falhou", (int)escrita, (unsigned)escritos);
  print_serial(linha);

  // fecha mesmo se a escrita falhou: e o f_close que descarrega o buffer e
  // atualiza o tamanho do arquivo na tabela
  FRESULT fechamento = f_close(&log_file);

  snprintf(linha, sizeof(linha), "f_close: %s (codigo %d)\r\n",
      fechamento == FR_OK ? "ok" : "falhou", (int)fechamento);
  print_serial(linha);
}

// Le em pedacos de 512 e imprime conforme le. O teto existe porque a fila de
// transmissao tem tamanho fixo: sem ele, um log grande seria descartado pela
// metade e voce leria um arquivo truncado sem saber.
#define LOG_READ_MAX_BYTES 2048

static char log_chunk[513];

static void command_log_read(void) {
  char linha[64];
  UINT lidos = 0;
  uint32_t total = 0;

  FRESULT abertura = f_open(&log_file, "log.txt", FA_READ);

  snprintf(linha, sizeof(linha), "f_open: %s (codigo %d)\r\n",
      abertura == FR_OK ? "ok" : "falhou", (int)abertura);
  print_serial(linha);

  if (abertura != FR_OK) {
    return;
  }

  snprintf(linha, sizeof(linha), "tamanho: %lu bytes\r\n",
      (unsigned long)f_size(&log_file));
  print_serial(linha);
  print_serial("-- conteudo --\r\n");

  do {
    FRESULT leitura = f_read(&log_file, log_chunk, sizeof(log_chunk) - 1, &lidos);

    if (leitura != FR_OK) {
      snprintf(linha, sizeof(linha), "f_read: falhou (codigo %d)\r\n", (int)leitura);
      print_serial(linha);
      break;
    }

    // byte de controle que nao seja fim de linha vira ponto, senao um arquivo
    // com lixo dentro embaralha o terminal. o zero encerraria a string cedo.
    for (UINT i = 0; i < lidos; i++) {
      char byte = log_chunk[i];
      if (byte != 0x0D && byte != 0x0A && (byte < 0x20 || byte == 0x7F)) {
        log_chunk[i] = 0x2E;
      }
    }
    log_chunk[lidos] = 0;

    print_serial(log_chunk);
    total += lidos;
  } while (lidos == sizeof(log_chunk) - 1 && total < LOG_READ_MAX_BYTES);

  if (total < f_size(&log_file)) {
    snprintf(linha, sizeof(linha), "\r\n-- cortado em %lu de %lu bytes --\r\n",
        (unsigned long)total, (unsigned long)f_size(&log_file));
    print_serial(linha);
  }

  FRESULT fechamento = f_close(&log_file);

  snprintf(linha, sizeof(linha), "f_close: %s (codigo %d)\r\n",
      fechamento == FR_OK ? "ok" : "falhou", (int)fechamento);
  print_serial(linha);
}

// ---------------------------------------------------------------------------
// TEMPORARIO - bancada. Dirige ou le um pino da porta B para conferir fiacao
// com o multimetro. Apagar junto com a linha do dispatch.
//
//   /pin 13 1   PB13 em alto      /pin 1 0   PB1 em baixo
//   /pin 14 r   le o nivel de PB14, com pull-up interno
//
// O pino sai da funcao alternativa ao virar saida, entao o periferico que o
// usava para de funcionar ate o proximo reset. PB13, PB14 e PB15 sao o SPI do
// cartao, PB1 e o CS, PB8 e PB9 sao o I2C, PB10 e o PWM do farol.
//
// Cuidado com o PB14 (MISO): dirigi-lo como saida com o cartao selecionado
// poe dois transmissores na mesma linha. Com o CS em alto o cartao solta a
// linha e nao ha disputa.
static void command_pin(const char *argumento) {
  char linha[64];
  uint32_t numero = 0;
  const char *p = argumento;

  while (*p >= 0x30 && *p <= 0x39) {
    numero = numero * 10 + (uint32_t)(*p - 0x30);
    p++;
  }

  if (p == argumento || numero > 15) {
    print_serial("uso: /pin <0-15> <0|1|r>\r\n");
    return;
  }

  while (*p == 0x20) {
    p++;
  }

  uint16_t mascara = (uint16_t)(1 << numero);

  if (*p == 0x72) {  // r de ler
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, mascara);

    snprintf(linha, sizeof(linha), "PB%lu le %d\r\n",
        (unsigned long)numero, gpio_get(GPIOB, mascara) ? 1 : 0);
  } else if (*p == 0x30 || *p == 0x31) {
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, mascara);

    if (*p == 0x31) {
      gpio_set(GPIOB, mascara);
    } else {
      gpio_clear(GPIOB, mascara);
    }

    snprintf(linha, sizeof(linha), "PB%lu em %s\r\n",
        (unsigned long)numero, *p == 0x31 ? "alto" : "baixo");
  } else {
    print_serial("uso: /pin <0-15> <0|1|r>\r\n");
    return;
  }

  print_serial(linha);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Configuracao programavel: /config <nome> <valor>
//
//   /config temperature_limit 24.5      limiar de temperatura, em graus Celsius
//
// Grava na flash, entao o valor sobrevive a reset e a desligar a placa. Com a
// flash virgem ou a assinatura errada, o load_config devolve os padroes.
//
// Quando aparecer a segunda configuracao isto pede uma tabela de nome ->
// campo, do mesmo jeito que os comandos do cartao SD. Com uma so, a escada
// ainda e mais legivel do que a tabela.
static void command_config(const char *argumento, config_t *config) {
  char linha[80];
  char nome[32];
  const char *p = argumento;
  size_t i = 0;

  // o nome vai ate o primeiro espaco
  while (*p != 0x20 && *p != 0 && i < sizeof(nome) - 1) {
    nome[i++] = *p++;
  }
  nome[i] = 0;

  while (*p == 0x20) {
    p++;
  }

  if (nome[0] == 0 || *p == 0) {
    print_serial("uso: /config <nome> <valor>\r\n");
    print_serial("  temperature_limit  limiar de temperatura, em graus Celsius (ex: 24.5)\r\n");
    return;
  }

  // o tecnico manda em graus Celsius, com ponto decimal: 24.5. quem converte
  // para milicelsius e aqui, porque milicelsius e unidade de codigo, nao de
  // painel. nada abaixo desta funcao volta a ver grau
  bool negativo = (*p == 0x2D);
  if (negativo) {
    p++;
  }

  int32_t inteiro = 0;
  const char *digitos = p;

  while (*p >= 0x30 && *p <= 0x39) {
    inteiro = inteiro * 10 + (int32_t)(*p - 0x30);
    p++;

    // seis digitos ja passam de qualquer temperatura concebivel, e mais que
    // isso estouraria o int32 depois de multiplicar por mil
    if (p - digitos > 6) {
      print_serial("valor invalido: numero grande demais\r\n");
      return;
    }
  }

  if (p == digitos) {
    print_serial("valor invalido: esperado graus Celsius, ex: 24.5\r\n");
    return;
  }

  // a parte decimal vale ate tres casas, que e o que milicelsius comporta
  int32_t milesimos = 0;

  if (*p == 0x2E) {
    p++;

    if (!(*p >= 0x30 && *p <= 0x39)) {
      print_serial("valor invalido: ponto decimal sem digito depois\r\n");
      return;
    }

    int32_t casa = 100;

    while (*p >= 0x30 && *p <= 0x39) {
      if (casa == 0) {
        print_serial("valor invalido: no maximo tres casas decimais\r\n");
        return;
      }
      milesimos += (int32_t)(*p - 0x30) * casa;
      casa /= 10;
      p++;
    }
  }

  // lixo depois do numero: nao da para adivinhar o que o tecnico quis
  if (*p != 0) {
    print_serial("valor invalido: esperado graus Celsius, ex: 24.5\r\n");
    return;
  }

  int32_t valor = inteiro * 1000 + milesimos;

  if (negativo) {
    valor = -valor;
  }

  if (strcmp(nome, "temperature_limit") != 0) {
    print_serial("configuracao desconhecida\r\n");
    print_serial("  temperature_limit  limiar de temperatura, em graus Celsius (ex: 24.5)\r\n");
    return;
  }

  config->temperature_millicelsius_limit = valor;

  // so anuncia depois de a flash confirmar: dizer que gravou sem ter gravado
  // seria pior do que falhar
  if (!save_config(*config)) {
    print_serial("config: gravacao na flash falhou\r\n");
    return;
  }

  // devolve em grau, na mesma unidade que ele digitou: confirmacao em
  // milicelsius obrigaria o tecnico a conferir a conta de cabeca
  int32_t absoluto = (valor < 0) ? -valor : valor;

  snprintf(linha, sizeof(linha),
      "config: temperature_limit = %s%ld.%03ld C\r\n",
      (valor < 0) ? "-" : "",
      (long)(absoluto / 1000), (long)(absoluto % 1000));
  print_serial(linha);
}
// ---------------------------------------------------------------------------

static void command_unknown(void) {
  print_serial("comando nao conhecido, consulte a tabela com /help\r\n");
}

void handle_command(const char* prompt, signal_state_t signal_state,
                    reset_cause_t last_reset_cause, config_t *config) {
  if (strcmp(prompt, "/hello") == 0) {
    command_hello();
  } else if (strcmp(prompt, "/help") == 0) {
    command_help();
  } else if (strcmp(prompt, "/status") == 0) {
    command_status(signal_state, last_reset_cause, config->temperature_millicelsius_limit);
  } else if (strcmp(prompt, "/adc_val") == 0) {
    command_adc_val();
  } else if (strcmp(prompt, "/battery_val") == 0) {
    command_battery_val();
  } else if (strcmp(prompt, "/shunt_current") == 0) {
    command_shunt_current();
  } else if (strcmp(prompt, "/lamp_status") == 0) {
    command_lamp_status();
  } else if (strcmp(prompt, "/temperature_raw") == 0) {
    command_temperature_raw(config->temperature_millicelsius_limit);
  } else if (strcmp(prompt, "/temperature") == 0) {
    command_temperature(config->temperature_millicelsius_limit);
  } else if (strcmp(prompt, "/fault_list") == 0) {
    command_fault_list();
  } else if (strncmp(prompt, "/fault ", 7) == 0) {
    command_fault(prompt + 7);
  } else if (strcmp(prompt, "/log") == 0) {
    command_log();
  } else if (strcmp(prompt, "/log_read") == 0) {
    command_log_read();
  } else if (strncmp(prompt, "/config ", 8) == 0) {
    command_config(prompt + 8, config);
  } else if (strncmp(prompt, "/pin ", 5) == 0) {  // TEMPORARIO - bancada
    command_pin(prompt + 5);
  } else {
    command_unknown();
  }
}
