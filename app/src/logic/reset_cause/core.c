#include "core.h"

const char * reset_cause_text(reset_cause_t reset_cause) {
  switch(reset_cause) {
    case RESET_LOW_POWER:
      return "LOW_POWER";
    case RESET_WINDOW_WATCHDOG:
      return "WINDOW_WATCHDOG";
    case RESET_WATCHDOG:
      return "WATCHDOG";
    case RESET_SOFTWARE:
      return "SOFTWARE";
    case RESET_POWER_ON:
      return "POWER_ON";
    case RESET_BUTTON_PRESS:
      return "BUTTON_PRESS";
    case RESET_BROWN_OUT:
      return "BROWN_OUT";
    default:
      return "UNKNOWN";
  }
}