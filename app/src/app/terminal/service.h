#ifndef APP_TERMINAL_H
#define APP_TERMINAL_H

#include "logic/turn_signal/core.h"

void handle_command(const char* prompt, signal_state_t signal_state);

#endif