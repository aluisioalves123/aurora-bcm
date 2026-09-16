#include "driver.h"

#include "hal/uart/driver.h"
#include "hal/systick/driver.h"
#include <stddef.h>

#define SD_CMD_PREFIX            0x40   // bits de start e transmission
#define SD_DUMMY_BYTE            0xFF
#define SD_R1_BUSY               0x80   // MSB em 1 = ainda nao respondeu
#define SD_R1_IDLE               0x01
#define SD_R1_READY              0x00
#define SD_LINE_BUSY             0x00
#define SD_LINE_IDLE             0xFF
#define SD_READY                 0xFE
#define SD_CMD_LENGTH            6
#define SD_NCR_MAX               10
#define SD_SLOW_CLOCK_DIVIDER    7

typedef enum {
  SD_CMD0_RESET,
  SD_CMD8_CHECK_INTERFACE,
  SD_CMD55_NEXT_IS_APP,
  SD_ACMD41_INITIALIZE,
  SD_CMD58_READ_OCR,
  SD_CMD13_HEALTH_CHECK,
  SD_COMMAND_COUNT
} sd_command_t;

typedef struct {
  uint8_t cmd[SD_CMD_LENGTH];
  uint8_t response_payload_length;
} sd_command_spec_t;

// tabela dos comandos possiveis no sd card
static const sd_command_spec_t SD_COMMANDS[SD_COMMAND_COUNT] = {
  [SD_CMD0_RESET]            = { { SD_CMD_PREFIX | 0,  0x00, 0x00, 0x00, 0x00, 0x95 }, 0 },
  [SD_CMD8_CHECK_INTERFACE]  = { { SD_CMD_PREFIX | 8,  0x00, 0x00, 0x01, 0xAA, 0x87 }, 4 },
  [SD_CMD55_NEXT_IS_APP]     = { { SD_CMD_PREFIX | 55, 0x00, 0x00, 0x00, 0x00, 0x01 }, 0 },
  [SD_ACMD41_INITIALIZE]     = { { SD_CMD_PREFIX | 41, 0x40, 0x00, 0x00, 0x00, 0x01 }, 0 },
  [SD_CMD58_READ_OCR]        = { { SD_CMD_PREFIX | 58, 0x00, 0x00, 0x00, 0x00, 0x01 }, 4 },
  [SD_CMD13_HEALTH_CHECK]    = { { SD_CMD_PREFIX | 13, 0x00, 0x00, 0x00, 0x00, 0x01 }, 1 },
};

void sd_wake_up(void) {
  // o cartão sd só começa a escutar depois de receber 10 bytes, 80 pulsos, pra inicializar, estamos mandando qualquer coisa aqui
  for (uint32_t i = 0; i < 10; i++) {
    spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  }
}

static void sd_select(void) {
  gpio_clear(SD_CARD_CS_PORT, SD_CARD_CS_PIN); // baixando o cs pra dizer pro sd que vai começar a comunicação
  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
}

static void sd_deselect(void) {
  gpio_set(SD_CARD_CS_PORT, SD_CARD_CS_PIN); // acabou a comunicação, levantando cs
  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
}

static sd_response_t send_cmd(sd_command_t command) {
  const sd_command_spec_t spec = SD_COMMANDS[command];

  sd_response_t response = {
    .r1 = 0xFF,
    .payload = { 0 }
  };

  sd_select();

  for (uint32_t i = 0; i < SD_CMD_LENGTH; i++) {
    spi_transfer(SD_CARD_SPI , spec.cmd[i]);
  }

  response.r1 = 0xFF; 
  for (uint32_t i = 0; i < SD_NCR_MAX && response.r1 & SD_R1_BUSY; i++) { // mascarando a resposta pra obter 0x00 quando a resposta for 0x01
    response.r1 = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  }

  for (uint32_t i = 0; i < spec.response_payload_length; i++) {
    response.payload[i] = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  }

  sd_deselect();

  return response;
}

bool spi_mode_config(void) {
  uint8_t response = 0x00;
  for(uint32_t i = 0; i < SD_NCR_MAX && response != SD_R1_IDLE; i++) { // tentar mais vezes pq o cartao as vezes demora pra confirmar e guarda estados antigos errados
    response = send_cmd(SD_CMD0_RESET).r1;
  }
  
  if (response == 0x01) return true;

  return false;
}

bool check_interface_condition(void) {
  sd_response_t response = send_cmd(SD_CMD8_CHECK_INTERFACE);

  // qualquer coisa diferente de 0x01 indica comando ilegal
  if (response.r1 != SD_R1_IDLE) return false;

  // os dois ultimos bytes sao o eco do argumento do cmd8: faixa de tensao e padrao de teste.
  // voltarem iguais prova que a comunicacao esta correta bit a bit nos dois sentidos
  if (response.payload[2] == 0x01 && response.payload[3] == 0xAA) return true;

  return false;
}

bool initialize_card(void) {
  uint8_t response = 0xFF;
  uint64_t started_waiting = get_ticks();
  uint64_t last_check = started_waiting;

  do {
    if (get_ticks() - last_check > 1000) {
      send_cmd(SD_CMD55_NEXT_IS_APP);
      response = send_cmd(SD_ACMD41_INITIALIZE).r1;
      last_check = get_ticks();
    }
  } while (get_ticks() - started_waiting < 10000 && response != SD_R1_READY);

  if (response == SD_R1_READY) return true;

  return false;
}

bool sd_health_check(void) {
  sd_response_t response = send_cmd(SD_CMD13_HEALTH_CHECK);

  if (response.r1 == SD_R1_READY && response.payload[0] == 0x00) return true;

  return false;
}

sd_addressing_type_t sd_addressing_type(void) {
  sd_response_t response = send_cmd(SD_CMD58_READ_OCR);

  if (response.payload[0] & 0x40) return SD_ADDRESSING_BLOCK;

  return SD_ADDRESSING_BYTE;
}

write_response_t write_block(const uint8_t * payload, const uint32_t block, sd_addressing_type_t sd_addressing_type) {
  if (sd_addressing_type == SD_ADDRESSING_BYTE) return SD_WRITE_NOT_IMPLEMENTED_ERROR;
  
  sd_select();
  
  // dividindo o numero do block em varios bytes pra poder enviar pro cartao sd qual bloco voce quer ler
  uint8_t cmd24[SD_CMD_LENGTH] = {
    SD_CMD_PREFIX | 24,
    (block >> 24) & 0xFF,
    (block >> 16) & 0xFF,
    (block >> 8)  & 0xFF,
    block         & 0xFF,
    0x01
  };

  for (uint32_t i = 0; i < SD_CMD_LENGTH; i++) {
    spi_transfer(SD_CARD_SPI , cmd24[i]);
  }

  uint8_t r1 = 0xFF; 
  for (uint32_t i = 0; i < SD_NCR_MAX && r1 & SD_R1_BUSY; i++) { // mascarando a resposta pra obter 0x00 quando a resposta for 0x01
    r1 = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  }

  if (r1 != SD_R1_READY) { 
    sd_deselect();
    return SD_WRITE_REJECTED_COMMAND;
  }

  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);

  spi_transfer(SD_CARD_SPI , SD_READY); // AVISANDO O CARTAO QUE VAI COMEÇAR O ENVIO DOS DADOS

  for (uint32_t i = 0; i < SD_BLOCK_SIZE; i++) { // mascarando a resposta pra obter 0x00 quando a resposta for 0x01
    spi_transfer(SD_CARD_SPI , payload[i]);
  }

  // dois bytes de crc pra encerrar a transmissao, o cartao sd exige isso
  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);

  uint8_t acceptance_byte = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE) & 0x1F; // so interessa os 5 bits de baixo

  // esperar o cartao sd terminar de gravar o conteudo
  uint8_t response = 0xFF;
  uint64_t started_waiting = get_ticks();
  do {
    response = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  } while (get_ticks() - started_waiting < 10000 && response == SD_LINE_BUSY);

  sd_deselect();

  if (acceptance_byte == 0x05) {
    return SD_WRITE_ACCEPTED;
  }  else if (acceptance_byte == 0x0B) {
    return SD_WRITE_CRC_ERROR;
  } else if (acceptance_byte == 0x0D) {
    return SD_WRITE_ERROR;
  } else {
    return SD_WRITE_UNKNOWN;
  }
}

read_response_t read_block(uint8_t * payload, const uint32_t block, sd_addressing_type_t sd_addressing_type) {
  if (sd_addressing_type == SD_ADDRESSING_BYTE) return SD_READ_NOT_IMPLEMENTED_ERROR;
  
  sd_select();
  
  // dividindo o numero do block em varios bytes pra poder enviar pro cartao sd qual bloco voce quer ler
  uint8_t cmd17[SD_CMD_LENGTH] = {
    SD_CMD_PREFIX | 17,
    (block >> 24) & 0xFF,
    (block >> 16) & 0xFF,
    (block >> 8)  & 0xFF,
    block         & 0xFF,
    0x01
  };

  for (uint32_t i = 0; i < SD_CMD_LENGTH; i++) {
    spi_transfer(SD_CARD_SPI , cmd17[i]);
  }

  uint8_t r1 = 0xFF; 
  for (uint32_t i = 0; i < SD_NCR_MAX && r1 & SD_R1_BUSY; i++) { // mascarando a resposta pra obter 0x00 quando a resposta for 0x01
    r1 = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  }

  if (r1 != SD_R1_READY) { 
    sd_deselect();
    return SD_READ_REJECTED_COMMAND;
  }

  // esperar pela confirmação do cartão que os dados estão prontos para começar a ler
  uint8_t response = 0xFF;
  uint64_t started_waiting = get_ticks();
  do {
    response = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  } while (get_ticks() - started_waiting < 10000 && response == SD_LINE_IDLE);

  bool response_byte_is_not_error = response & 0xF0; // essa mascara descobre se os 4 bits de cima estao zerados, se sim, é pq o response indica um erro de leitura
  if (response != SD_READY && response_byte_is_not_error) {
    sd_deselect();
    return SD_READ_TIMEOUT;
  } else if (response != SD_READY) {
    sd_deselect();
    return SD_READ_ERROR;
  }

  for (uint32_t i = 0; i < SD_BLOCK_SIZE; i++) {
    payload[i] = spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  }

  // dois bytes de crc pra encerrar a transmissao, o cartao sd exige isso
  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);
  spi_transfer(SD_CARD_SPI , SD_DUMMY_BYTE);

  sd_deselect();

  return SD_READ_ACCEPTED;
}

void sd_clock_speed_up(void) {
  sd_addressing_type_t sd_address_type = sd_addressing_type();
  for (int8_t i = 6; i >=0; i--) {
    spi_set_clock_divider(i);
    uint8_t response[SD_BLOCK_SIZE];

    read_response_t read_response = read_block(response, 0, sd_address_type);

    print_value("dividr", i);

    if (read_response != SD_READ_ACCEPTED) {
      spi_set_clock_divider(i + 1);
      return;
    }

    if (response[510] != 0x55 || response[511] != 0xAA) {
      spi_set_clock_divider(i + 1);
      return;
    }
  }
}


void sd_clock_reset_speed(void) {
  spi_set_clock_divider(SD_SLOW_CLOCK_DIVIDER);
}