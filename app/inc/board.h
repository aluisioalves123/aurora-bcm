#ifndef INC_BOARD_H
#define INC_BOARD_H

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

// Mapa do hardware. Se a fiacao ou a PCB mudar, este e o unico arquivo
// que precisa mudar junto.

// --- Farois ---
#define LEDS_PORT                     (GPIOA)
#define TURN_SIGNAL_RIGHT_LED_PIN     (GPIO5)
#define TURN_SIGNAL_LEFT_LED_PIN      (GPIO6)

// --- Botoes (ativos em baixo, com pull-up interno) ---
#define TURN_SIGNAL_RIGHT_BUTTON_PORT (GPIOB)
#define TURN_SIGNAL_RIGHT_BUTTON_PIN  (GPIO6)

#define TURN_SIGNAL_LEFT_BUTTON_PORT  (GPIOC)
#define TURN_SIGNAL_LEFT_BUTTON_PIN   (GPIO7)

#define HAZARD_BUTTON_PORT            (GPIOA)
#define HAZARD_BUTTON_PIN             (GPIO9)

#define SERVICE_LIGHT_BUTTON_PORT     (GPIOC)
#define SERVICE_LIGHT_BUTTON_PIN      (GPIO13)

// --- Console de diagnostico ---
// USART2 no PA2/PA3: na Nucleo-F446RE esses dois pinos vao para o ST-LINK,
// que os entrega ao PC como porta serial virtual pela mesma USB da gravacao.
#define CONSOLE_UART                  (USART2)
#define CONSOLE_UART_PORT             (GPIOA)
#define CONSOLE_UART_TX_PIN           (GPIO2)
#define CONSOLE_UART_RX_PIN           (GPIO3)

#endif // INC_BOARD_H
