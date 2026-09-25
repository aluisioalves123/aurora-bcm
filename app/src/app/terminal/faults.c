// Comandos que respondem o que os sensores ja disseram antes.
//
// A tabela de falhas guarda historico: contador de ocorrencias, estado atual e
// o instante da ultima vez. O contador nunca e zerado ao limpar, de proposito —
// falha intermitente e a mais dificil de achar, e e o historico que a denuncia.

#include "app/terminal/commands.h"

#include <stdio.h>
#include <string.h>

#include "hal/uart/driver.h"
#include "logic/fault_table/core.h"

void command_fault_list(const char *argument, console_t *console) {
  (void)argument;
  (void)console;

  char linha[64];
  int i;

  print_serial("tipos de falha que podem ser consultados:\r\n");

  for (i = 0; i < FAULT_COUNT; i++) {
    snprintf(linha, sizeof(linha), "  %s\r\n", fault_code_name((fault_code_t)i));
    print_serial(linha);
  }

  print_serial("use /fault <tipo> para ver o estado de um deles\r\n");
}

void command_fault(const char *argument, console_t *console) {
  (void)console;

  char linha[128];
  int i;

  for (i = 0; i < FAULT_COUNT; i++) {
    if (strcmp(argument, fault_code_name((fault_code_t)i)) == 0) {
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
