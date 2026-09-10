#include "core.h"

const char * reset_cause_text(reset_cause_t reset_cause) {
  switch(reset_cause) {
    case LOW_POWER:
      return "LOW_POWER";
    case WINDOW_WATCHDOG:
      return "WINDOW_WATCHDOG";
    case WATCHDOG:
      return "WATCHDOG";
    case SOFTWARE:
      return "SOFTWARE";
    case POWER_ON:
      return "POWER_ON";
    case BUTTON_PRESS:
      return "BUTTON_PRESS";
    case BROWN_OUT:
      return "BROWN_OUT";
    default:
      return "UNKNOWN";
  }
}