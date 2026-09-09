#ifndef INC_LOGIC_FAULT_TABLE_H
#define INC_LOGIC_FAULT_TABLE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  FAULT_LAMP_OPEN,
  FAULT_TEMPERATURE_HIGH,
  FAULT_BATTERY_LOW,
  FAULT_RX_BYTE_LOST,
  FAULT_TX_BYTE_LOST,
  FAULT_SENSOR_NOT_RESPONDING,
  FAULT_COUNT // só serve pra contar quantos tipos de falha existem
} fault_code_t;

typedef struct {
  uint32_t count;
  bool active;
  uint64_t last_seen_at;
} fault_entry_t;

void fault_report(fault_code_t code, uint64_t now);

fault_entry_t fault_get(fault_code_t code);

void fault_clear(fault_code_t code);

const char * fault_code_name(fault_code_t fault_code);

#endif