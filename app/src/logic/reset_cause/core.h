#ifndef LOGIC_RESET_CAUSE_H
#define LOGIC_RESET_CAUSE_H

#include <stdint.h>

typedef enum {
  LOW_POWER,
  WINDOW_WATCHDOG,
  WATCHDOG,
  SOFTWARE,
  POWER_ON,
  BUTTON_PRESS,
  BROWN_OUT,
  UNKNOWN
} reset_cause_t; 

const char * reset_cause_text(reset_cause_t reset_cause);

#endif
