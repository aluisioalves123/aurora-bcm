#ifndef HAL_RESET_CAUSE_H
#define HAL_RESET_CAUSE_H

#include <stdint.h>
#include "logic/reset_cause/core.h"

uint32_t read_and_clear_reset_cause_register(void);

reset_cause_t reset_cause(uint32_t csr);

#endif
