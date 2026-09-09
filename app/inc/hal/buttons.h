#ifndef INC_HAL_BUTTONS_H
#define INC_HAL_BUTTONS_H

#include "logic/buttons.h"

void buttons_setup(void);

button_states_t read_buttons(void);

#endif
