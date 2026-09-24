#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "board.h"

typedef enum {
  CAN_SEND_OK,
  CAN_SEND_MAILBOX_FULL,
  CAN_SEND_INVALID_LENGTH
} can_send_result_t;

typedef struct {
  uint32_t id;
  bool extended;
  uint8_t length;
  uint8_t data[8];
} can_frame_t;

void can_setup(void);

can_send_result_t can_send(can_frame_t can_frame);

bool can_read(can_frame_t *frame);

#endif
