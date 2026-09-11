#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "driver.h"
#include "board.h"

void spi_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_SPI2);

  gpio_mode_setup(
    SPI_PORT,
    GPIO_MODE_AF,
    GPIO_PUPD_NONE,
    SPI_SCK_PIN | SPI_MOSI_PIN | SPI_MISO_PIN
  );

  gpio_mode_setup(
    SD_CARD_CS_PORT,
    GPIO_MODE_OUTPUT,
    GPIO_PUPD_NONE,
    SD_CARD_CS_PIN
  );

  gpio_set_af(SPI_PORT, GPIO_AF5, SPI_SCK_PIN);
  gpio_set_af(SPI_PORT, GPIO_AF5, SPI_MOSI_PIN);
  gpio_set_af(SPI_PORT, GPIO_AF5, SPI_MISO_PIN);
  
  spi_disable(SPI2);

  spi_init_master(
    SPI2,
    SPI_CR1_BAUDRATE_FPCLK_DIV_128, // com o clock do barramento em 45MHz, a divisão precisa ser por 128 pra ficar abaixo de 400KHz, que é o que o cartão aceita
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, // padrão 0, padrão cartão sd
    SPI_CR1_CPHA_CLK_TRANSITION_1, // padrão cartão sd
    SPI_CR1_DFF_8BIT, // padrão cartão sd
    SPI_CR1_MSBFIRST // padrão cartão sd
  );

  spi_enable(SPI2);
  gpio_set(SD_CARD_CS_PORT, SD_CARD_CS_PIN); // levantando a linha cs pra nao deixar o cartao sd selecionado
}

uint16_t spi_transfer(uint32_t spi, uint16_t data) {
  uint16_t response = spi_xfer(spi, data);
  return response;
}