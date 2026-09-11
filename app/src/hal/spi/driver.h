#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "board.h"

void spi_setup(void);

uint16_t spi_transfer(uint32_t spi, uint16_t data);

#endif
