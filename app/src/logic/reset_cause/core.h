#ifndef LOGIC_RESET_CAUSE_H
#define LOGIC_RESET_CAUSE_H

#include <stdint.h>

typedef enum {
  RESET_LOW_POWER,
  RESET_WINDOW_WATCHDOG,
  RESET_WATCHDOG,
  RESET_SOFTWARE,
  RESET_POWER_ON,
  RESET_BUTTON_PRESS,
  RESET_BROWN_OUT,
  RESET_UNKNOWN
} reset_cause_t; 

const char * reset_cause_text(reset_cause_t reset_cause);

#endif
