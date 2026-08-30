#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "board.h"
#include "hal/uart.h"
#include "logic/ring_buffer.h"

// byte que chegou com o buffer cheio e nao coube. volatile porque quem
// escreve e a interrupcao: sem isso o compilador poderia guardar o valor
// num registrador e quem le nunca veria o contador subir.
// perder byte pode acontecer; perder em silencio, nao.
static volatile uint32_t lost_bytes = 0;

// o buffer de recepcao: a interrupcao escreve, o main tira pelo read_serial
static ring_buffer_t rx_buffer = { .data = { 0 }, .head = 0, .tail = 0 };

// o buffer de transmissao: quem quer mandar enfileira aqui, a interrupcao
// tira um byte por vez conforme o transmissor for ficando livre
static ring_buffer_t tx_buffer = { .data = { 0 }, .head = 0, .tail = 0 };

void uart_setup(void) {
  // todo modulo habilita o que usa por completo: porta, pino e clock.
  // o clock do GPIOA fica repetido em relacao ao lamps_setup de proposito:
  // o console nao pode depender de uma linha que mora no modulo dos farois.
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART2);

  // PA2 e PA3 em funcao alternativa AF7, que e onde o USART2 aparece nesses
  // dois pinos. RX ja fica configurado agora, mesmo sem uso, para o pino nao
  // ficar pela metade quando a recepcao entrar.
  gpio_mode_setup(
    CONSOLE_UART_PORT,
    GPIO_MODE_AF,
    GPIO_PUPD_NONE,
    CONSOLE_UART_TX_PIN | CONSOLE_UART_RX_PIN
  );
  gpio_set_af(
    CONSOLE_UART_PORT,
    GPIO_AF7,
    CONSOLE_UART_TX_PIN | CONSOLE_UART_RX_PIN
  );

  // 8 bits de dados, 1 stop bit, sem paridade, sem controle de fluxo
  usart_set_baudrate(CONSOLE_UART, 115200);
  usart_set_databits(CONSOLE_UART, 8);
  usart_set_stopbits(CONSOLE_UART, USART_STOPBITS_1);
  usart_set_parity(CONSOLE_UART, USART_PARITY_NONE);
  usart_set_flow_control(CONSOLE_UART, USART_FLOWCONTROL_NONE);

  // transmissao e recepcao
  usart_set_mode(CONSOLE_UART, USART_MODE_TX_RX);

  // o periferico avisa por interrupcao quando um byte chega, em vez de o
  // main ter que ficar perguntando. o NVIC precisa autorizar essa linha
  // separadamente: uma coisa e o periferico querer interromper, outra e o
  // nucleo aceitar a interrupcao
  usart_enable_rx_interrupt(CONSOLE_UART);
  usart_enable_tx_interrupt(CONSOLE_UART);
  nvic_enable_irq(NVIC_USART2_IRQ);

  usart_enable(CONSOLE_UART);
}

void usart2_isr(void) {
  // o vetor do USART2 e um so para todas as causas do periferico: byte
  // recebido, transmissor livre, erro de enquadramento. por isso cada
  // tratativa testa o proprio flag antes de fazer qualquer coisa
  if (usart_get_flag(CONSOLE_UART, USART_FLAG_RXNE)) {
    // ler o registrador de dados e o que abaixa o RXNE. sem essa leitura a
    // interrupcao voltaria a disparar sem parar, no mesmo byte
    if (!ring_buffer_put(&rx_buffer, (uint8_t)usart_recv(CONSOLE_UART))) {
      lost_bytes++;
    }
  }

  if (usart_get_flag(CONSOLE_UART, USART_FLAG_TXE)) {
    read_result_t outgoing = ring_buffer_get(&tx_buffer);

    if (outgoing.success) {
      // escrever no registrador de dados e o que abaixa o TXE
      usart_send(CONSOLE_UART, outgoing.value);
    } else {
      // fila vazia: desliga o pedido. o TXE fica em 1 o tempo todo com o
      // transmissor livre, entao deixar habilitado seria disparar sem parar
      usart_disable_tx_interrupt(CONSOLE_UART);
    }
  }
}

void print_serial(const char *frase) {
  uint32_t i = 0;

  // a frase acaba no byte 0, que e o terminador que o compilador poe no
  // fim de todo texto entre aspas duplas
  while (frase[i] != '\0') {
    ring_buffer_put(&tx_buffer, (uint8_t)frase[i]);
    i++;
  }

  // a ISR se desabilita quando esvazia a fila, entao quem enfileira
  // precisa religar o pedido, senao ninguem vem buscar
  usart_enable_tx_interrupt(CONSOLE_UART);
}

read_result_t read_serial(void) {
  return ring_buffer_get(&rx_buffer);
}
