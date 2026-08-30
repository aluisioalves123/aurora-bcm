#include "logic/message.h"

message_t check_for_messages(message_t current, read_result_t incoming) {
  message_t next = current;
  char received;

  // complete vale por um ciclo so: quem chamou ja viu a frase anterior
  next.complete = false;

  if (!incoming.success) {
    return next; // nao chegou nada
  }

  received = (char)incoming.value;

  if (received == '\r' || received == '\n') {
    // frase vazia (o \n que vem logo depois do \r, ou dois Enter seguidos)
    // nao vira mensagem: nao ha o que entregar
    if (next.length > 0) {
      next.message[next.length] = '\0';
      next.complete = true;
      next.length = 0;
    }

    return next;
  }

  // frase maior que o buffer: o excesso e descartado e a frase so fecha
  // quando o \r ou \n chegar
  if (next.length < MESSAGE_MAX_LENGTH - 1) {
    next.message[next.length] = received;
    next.length++;
  }

  return next;
}
