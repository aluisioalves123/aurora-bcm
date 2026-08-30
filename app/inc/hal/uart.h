#ifndef INC_HAL_UART_H
#define INC_HAL_UART_H

#include "logic/ring_buffer.h"

// Console de diagnostico da etapa 05, no USART2: 115200 8N1, sem controle
// de fluxo, so transmissao por enquanto.
//
// Tem que ser chamada depois do rcc_setup. O divisor do baud rate e
// calculado a partir da frequencia do APB1 no momento da chamada, e essa
// frequencia so esta correta depois que o PLL foi configurado.
void uart_setup(void);

// enfileira a frase no buffer de transmissao e volta na hora. quem manda
// os bytes para o fio e a interrupcao, um por vez, conforme o
// transmissor fica livre. nao bloqueia.
void print_serial(const char *frase);

// tira um byte do buffer circular, o que a interrupcao ja recebeu. nao
// espera: se nao houver nada, volta com success = false
read_result_t read_serial(void);

#endif // INC_HAL_UART_H
