#include "logic/fault_table.h"

static fault_entry_t faults[FAULT_COUNT];

void fault_report(fault_code_t code, uint64_t now) {
  faults[code].active = true;
  faults[code].last_seen_at = now;
  faults[code].count++;
}

fault_entry_t fault_get(fault_code_t code) {
  return faults[code];
}

void fault_clear(fault_code_t code) {
  faults[code].active = false;
}

const char * fault_code_name(fault_code_t fault_code) {
  switch(fault_code) {
    case FAULT_LAMP_OPEN:
      return "FAULT_LAMP_OPEN";
    case FAULT_TEMPERATURE_HIGH:
      return "FAULT_TEMPERATURE_HIGH";
    case FAULT_BATTERY_LOW:
      return "FAULT_BATTERY_LOW";
    case FAULT_RX_BYTE_LOST:
      return "FAULT_RX_BYTE_LOST";
    case FAULT_TX_BYTE_LOST:
      return "FAULT_TX_BYTE_LOST";
    case FAULT_SENSOR_NOT_RESPONDING:
      return "FAULT_SENSOR_NOT_RESPONDING";
    default:
      return "UNKNOWN";
  }
}
