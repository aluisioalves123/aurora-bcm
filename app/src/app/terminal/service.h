#ifndef APP_TERMINAL_H
#define APP_TERMINAL_H

#include "logic/turn_signal/core.h"
#include "hal/reset_cause/driver.h"

void handle_command(const char* prompt, signal_state_t signal_state, reset_cause_t last_reset_cause);

#endif