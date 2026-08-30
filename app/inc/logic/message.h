#ifndef INC_LOGIC_MESSAGE_H
#define INC_LOGIC_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "logic/ring_buffer.h"

#define MESSAGE_MAX_LENGTH 64

typedef struct {
  char message[MESSAGE_MAX_LENGTH];
  uint32_t length;  // quanto da frase ja foi montado
  bool complete;    // verdadeiro so no ciclo em que o \r ou \n chegou
} message_t;

// funcao pura: recebe o estado da frase e o byte que acabou de chegar, e
// devolve o estado novo. nao le nem escreve global, nao toca hardware e
// nao sabe que existe UART. quem le o byte e o main.
message_t check_for_messages(message_t current, read_result_t incoming);

#endif // INC_LOGIC_MESSAGE_H
