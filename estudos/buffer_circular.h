/*
 * buffer_circular.h — interface do buffer circular.
 *
 * Autoria: declarações transcritas do código de Aluisio Cunha.
 * Este arquivo não contém lógica; existe só para o arquivo de teste
 * enxergar o que buffer_circular.c implementa.
 */

#ifndef BUFFER_CIRCULAR_H
#define BUFFER_CIRCULAR_H

#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE 8

typedef struct {
    bool success;
    uint8_t value;
} resultado_leitura_t;

extern volatile uint32_t head;
extern volatile uint32_t tail;

bool inserir_no_buffer(uint8_t letra);
resultado_leitura_t ler_do_buffer(void);

#endif /* BUFFER_CIRCULAR_H */
