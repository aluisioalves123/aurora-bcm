// O mecanismo do console: a tabela de comandos, o /help que se imprime dela, e
// o despachante. Nenhum comando mora aqui — eles estao em readings.c, faults.c,
// storage.c e config.c, e chegam por app/terminal/commands.h.

#include "app/terminal/service.h"
#include "app/terminal/commands.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hal/uart/driver.h"

// definido depois da tabela, porque le a tabela para se imprimir
static void command_help(const char *argument, console_t *console);

// ---------------------------------------------------------------------------
// A tabela de comandos. Acrescentar um comando e uma linha aqui, e o /help
// acompanha sozinho — ele se imprime a partir desta tabela, entao nao existe
// lista de ajuda para manter em dia num canto separado.
//
// argument com NULL diz que o comando nao leva argumento. Nao existe um campo
// separado dizendo isso, de proposito: dois campos poderiam se contradizer.
static const command_t COMMANDS[] = {
  { "/help",        NULL,             command_help,        "esta lista" },
  { "/status",      NULL,             command_status,      "estado geral da placa" },
  { "/battery_val", NULL,             command_battery_val, "tensao da bateria, em volts" },
  { "/lamp_status", NULL,             command_lamp_status, "diagnostico da lampada da seta direita" },
  { "/temperature", NULL,             command_temperature, "temperatura em graus Celsius" },
  { "/fault_list",  NULL,             command_fault_list,  "tipos de falha que podem ser consultados" },
  { "/fault",       "<tipo>",         command_fault,       "estado de uma falha: ativa, ocorrencias e quando" },
  { "/log",         NULL,             command_log,         "acrescenta uma linha em log.txt no cartao" },
  { "/log_read",    NULL,             command_log_read,    "mostra o conteudo de log.txt" },
  { "/config",      "<nome> <valor>", command_config,      "grava uma configuracao na flash (ex: /config temperature_limit 24.5)" },
};

#define COMMAND_COUNT (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

static void command_help(const char *argument, console_t *console) {
  (void)argument;
  (void)console;

  char linha[128];
  char chamada[40];

  print_serial("comandos disponiveis:\r\n");

  for (size_t i = 0; i < COMMAND_COUNT; i++) {
    snprintf(chamada, sizeof(chamada), "%s%s%s",
        COMMANDS[i].name,
        COMMANDS[i].argument ? " " : "",
        COMMANDS[i].argument ? COMMANDS[i].argument : "");

    snprintf(linha, sizeof(linha), "  %-28s %s\r\n", chamada, COMMANDS[i].help);
    print_serial(linha);
  }
}

void handle_command(const char* prompt, signal_state_t signal_state,
                    reset_cause_t last_reset_cause, config_t *config) {
  console_t console = {
    .signal_state     = signal_state,
    .last_reset_cause = last_reset_cause,
    .config           = config
  };

  for (size_t i = 0; i < COMMAND_COUNT; i++) {
    const command_t *comando = &COMMANDS[i];

    if (comando->argument == NULL) {
      if (strcmp(prompt, comando->name) == 0) {
        comando->handler(NULL, &console);
        return;
      }
      continue;
    }

    // com argumento, o nome precisa vir seguido de espaco. e isso que separa
    // "/fault FAULT_LAMP_OPEN" de "/fault_list", que comeca igual
    size_t tamanho = strlen(comando->name);

    if (strncmp(prompt, comando->name, tamanho) == 0 && prompt[tamanho] == 0x20) {
      const char *valor = prompt + tamanho + 1;

      while (*valor == 0x20) {
        valor++;
      }

      comando->handler(valor, &console);
      return;
    }
  }

  print_serial("comando nao conhecido, consulte a tabela com /help\r\n");
}
