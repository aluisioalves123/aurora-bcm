#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <hal/systick.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "hal/i2c.h"
#include "board.h"

void i2c_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_I2C1);

  gpio_mode_setup(
    LM75_SCL_PORT,
    GPIO_MODE_AF,
    GPIO_PUPD_NONE,
    LM75_SCL_PIN
  );

  gpio_mode_setup(
    LM75_SDA_PORT,
    GPIO_MODE_AF,
    GPIO_PUPD_NONE,
    LM75_SDA_PIN
  );

  gpio_set_output_options(LM75_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, LM75_SCL_PIN);
  gpio_set_output_options(LM75_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, LM75_SDA_PIN);

  gpio_set_af(LM75_SCL_PORT, GPIO_AF4, LM75_SCL_PIN);
  gpio_set_af(LM75_SDA_PORT, GPIO_AF4, LM75_SDA_PIN);
  
  i2c_peripheral_disable(I2C1);
  i2c_set_speed(I2C1, i2c_speed_sm_100k, rcc_apb1_frequency / 1000000);

  i2c_peripheral_enable(I2C1);

}

bool i2c_write_with_timeout(uint32_t i2c, int i2c_address, const uint8_t *data, size_t data_length) {
  uint64_t started_waiting = get_ticks();
  while ((I2C_SR2(i2c) & I2C_SR2_BUSY)) {
    if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
	}

	i2c_send_start(i2c);

	/* Wait for the end of the start condition, master mode selected, and BUSY bit set */
  started_waiting = get_ticks();
	while ( !( (I2C_SR1(i2c) & I2C_SR1_SB)
		&& (I2C_SR2(i2c) & I2C_SR2_MSL)
		&& (I2C_SR2(i2c) & I2C_SR2_BUSY) )) {
      if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
    };

	i2c_send_7bit_address(i2c, i2c_address, I2C_WRITE);

	/* Waiting for address is transferred. */
  started_waiting = get_ticks();
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR)) {
    if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
  };

	/* Clearing ADDR condition sequence. */
	(void)I2C_SR2(i2c);

	for (size_t i = 0; i < data_length; i++) {
		i2c_send_data(i2c, data[i]);
    started_waiting = get_ticks();
		while (!(I2C_SR1(i2c) & (I2C_SR1_BTF))) {
      if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
    };
	}
  
  return true;
}

bool i2c_read_with_timeout(uint32_t i2c, int i2c_address, uint8_t *response, size_t response_length) {
  i2c_send_start(i2c);
	i2c_enable_ack(i2c);

  uint64_t started_waiting = get_ticks();
	/* Wait for the end of the start condition, master mode selected, and BUSY bit set */
	while ( !( (I2C_SR1(i2c) & I2C_SR1_SB)
		&& (I2C_SR2(i2c) & I2C_SR2_MSL)
		&& (I2C_SR2(i2c) & I2C_SR2_BUSY) )) {
      if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
    };

	i2c_send_7bit_address(i2c, i2c_address, I2C_READ);

	/* Waiting for address is transferred. */
  started_waiting = get_ticks();
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR)) {
    if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
  };
	/* Clearing ADDR condition sequence. */
	(void)I2C_SR2(i2c);

	for (size_t i = 0; i < response_length; ++i) {
		if (i == response_length - 1) {
			i2c_disable_ack(i2c);
		}
    started_waiting = get_ticks();
		while (!(I2C_SR1(i2c) & I2C_SR1_RxNE)) {
      if (get_ticks() - started_waiting >= I2C_TIMEOUT) return false;
    };
		response[i] = i2c_get_data(i2c);
	}
	i2c_send_stop(i2c);

	return true;
}

bool i2c_recover(uint32_t i2c) {
  i2c_send_stop(i2c);
  rcc_periph_reset_pulse(RST_I2C1);
  i2c_setup();
  return false;
}

bool i2c_transfer_with_timeout(
  uint32_t i2c,
  uint8_t i2c_address,
  const uint8_t *write_buffer,
  size_t write_length, 
  uint8_t *read_buffer,
  size_t read_length
) {
  if(write_buffer) {
    if(!i2c_write_with_timeout(i2c, i2c_address, write_buffer, write_length)) { return i2c_recover(i2c); };
  }
  
  if (read_buffer) {
    if(!i2c_read_with_timeout(i2c, i2c_address, read_buffer, read_length)) { return i2c_recover(i2c); };
  } else {
    i2c_send_stop(i2c);
  }

  return true;
}