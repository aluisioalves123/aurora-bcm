#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/can.h>
#include <stddef.h>

#include "driver.h"

#include "board.h"

void can_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_CAN1);

  gpio_mode_setup(
    CAN_PORT,
    GPIO_MODE_AF,
    GPIO_PUPD_NONE,
    CAN_TX_PIN | CAN_RX_PIN
  );

  gpio_set_af(CAN_PORT, GPIO_AF9, CAN_TX_PIN);
  gpio_set_af(CAN_PORT, GPIO_AF9, CAN_RX_PIN);

  can_init(CAN1, false, true, false, false, false, false, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_14TQ, CAN_BTR_TS2_3TQ , CAN_PRESCALER, false, false);
  can_filter_id_mask_32bit_init(0, 0, 0, 0, true);
}

can_send_result_t can_send(const can_frame_t frame) {
  if (frame.length > 8) {
    return CAN_SEND_INVALID_LENGTH;
  };

  int16_t can_transmit_result = 
    can_transmit(CAN1, frame.id, frame.extended, false, frame.length, (uint8_t *)frame.data);

  if (can_transmit_result >= 0) {
    return CAN_SEND_OK;
  }

  return CAN_SEND_MAILBOX_FULL;
}

bool can_read(can_frame_t *frame) {
  uint32_t messages_on_inbox = can_fifo_pending(CAN1, 0);
  bool rtr_descartado;
  uint8_t fmi_descartado;

  if (messages_on_inbox > 0) {
    can_receive(
      CAN1,
      0,
      true,
      &frame->id,
      &frame->extended,
      &rtr_descartado,
      &fmi_descartado,
      &frame->length,
      frame->data,
      NULL
    );

    return true;
  }

  return false;
}