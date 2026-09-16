#ifndef HAL_SD_CARD_H
#define HAL_SD_CARD_H

#include "hal/spi/driver.h"
#include "board.h"

// tamanho do bloco: faz parte do contrato, quem chama write_block e
// read_block precisa dimensionar o buffer por ele
#define SD_BLOCK_SIZE 512

typedef struct {
  uint8_t r1;
  uint8_t payload[4];
} sd_response_t;

typedef enum {
  SD_WRITE_ACCEPTED,
  SD_WRITE_CRC_ERROR,
  SD_WRITE_ERROR,
  SD_WRITE_REJECTED_COMMAND,
  SD_WRITE_NOT_IMPLEMENTED_ERROR,
  SD_WRITE_UNKNOWN
} write_response_t;

typedef enum {
  SD_READ_ACCEPTED,
  SD_READ_TIMEOUT,
  SD_READ_ERROR,
  SD_READ_REJECTED_COMMAND,
  SD_READ_NOT_IMPLEMENTED_ERROR,
  SD_READ_UNKNOWN
} read_response_t;

typedef enum {
  SD_ADDRESSING_BLOCK,
  SD_ADDRESSING_BYTE
} sd_addressing_type_t;

void sd_wake_up(void);

bool spi_mode_config(void);

bool check_interface_condition(void);

bool initialize_card(void);

bool sd_health_check(void);

void sd_clock_speed_up(void);

void sd_clock_reset_speed(void);

sd_addressing_type_t sd_addressing_type(void);

write_response_t write_block(const uint8_t * payload, const uint32_t block, sd_addressing_type_t sd_addressing_type);

read_response_t read_block(uint8_t * payload, const uint32_t block, sd_addressing_type_t sd_addressing_type);

#endif