#ifndef APP_TERMINAL_H
#define APP_TERMINAL_H

#include "logic/turn_signal/core.h"
#include "hal/reset_cause/driver.h"
#include "logic/config/core.h"

// o config entra por ponteiro porque o /config escreve nele: o valor vigente
// mora no main, e o terminal e quem o usuario usa para mexer nele
void handle_command(const char* prompt, signal_state_t signal_state,
                    reset_cause_t last_reset_cause, config_t *config);

#endif