#ifndef APP_TERMINAL_COMMANDS_H
#define APP_TERMINAL_COMMANDS_H

#include "app/terminal/service.h"

// Contrato entre o despachante e os comandos.
//
// Os comandos moram em arquivos separados por assunto — readings.c, faults.c,
// storage.c, config.c — e a tabela que os reune fica no service.c. Este header
// e o que os liga: os tipos que a tabela usa e as declaracoes dos handlers.

// Tudo que um comando possa precisar do resto do firmware viaja aqui dentro.
//
// Existe para os comandos terem assinatura unica: sem isso cada um pediria os
// seus proprios parametros, e uma tabela nao consegue guardar ponteiros para
// funcoes de tipos diferentes. Comando que nao usa nada tambem recebe — sai
// mais barato do que ter duas familias de comando.
typedef struct {
  signal_state_t  signal_state;
  reset_cause_t   last_reset_cause;
  config_t       *config;
} console_t;

// argument e NULL nos comandos que nao levam argumento
typedef void (*command_handler_t)(const char *argument, console_t *console);

typedef struct {
  const char        *name;
  const char        *argument;  // NULL = comando sem argumento; senao, o que mostrar no /help
  command_handler_t  handler;
  const char        *help;
} command_t;

// readings.c — o que os sensores estao dizendo agora
void command_status(const char *argument, console_t *console);
void command_battery_val(const char *argument, console_t *console);
void command_lamp_status(const char *argument, console_t *console);
void command_temperature(const char *argument, console_t *console);

// faults.c — o que os sensores ja disseram antes
void command_fault_list(const char *argument, console_t *console);
void command_fault(const char *argument, console_t *console);

// storage.c — o cartao
void command_log(const char *argument, console_t *console);
void command_log_read(const char *argument, console_t *console);

// config.c — o que sobrevive ao desligamento
void command_config(const char *argument, console_t *console);

#endif // APP_TERMINAL_COMMANDS_H
