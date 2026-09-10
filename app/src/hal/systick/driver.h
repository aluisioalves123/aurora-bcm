#ifndef HAL_SYSTICK_H
#define HAL_SYSTICK_H

#include <stdint.h>

#define CPU_FREQ     (180000000)
#define SYSTICK_FREQ (1000)

void systick_setup(void);
uint64_t get_ticks(void);

#endif // HAL_SYSTICK_H
