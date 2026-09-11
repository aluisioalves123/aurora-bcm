#ifndef HAL_SD_CARD_H
#define HAL_SD_CARD_H

#include "hal/spi/driver.h"
#include "board.h"

typedef struct {
  uint8_t r1;
  uint8_t payload[4];
} sd_response_t;

typedef enum {
  SD_WRITE_ACCEPTED,
  SD_WRITE_CRC_ERROR,
  SD_WRITE_ERROR,
  SD_WRITE_REJECTED_COMMAND,
  SD_WRITE_UNKNOWN
} write_response_t;

void wake_up(void);

uint8_t spi_mode_config(void);

sd_response_t check_interface_condition(void);

bool initialize_card(void);

bool card_uses_block_addressing(void);

write_response_t write_block(const uint8_t * payload, const uint32_t block);

#endif