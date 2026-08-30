/*
 * ring_buffer.h — interface do buffer circular.
 *
 * Autoria: declarações transcritas do código de Aluisio Cunha.
 * Este arquivo não contém lógica; existe só para quem usa o buffer
 * enxergar o que ring_buffer.c implementa.
 */

#ifndef INC_LOGIC_RING_BUFFER_H
#define INC_LOGIC_RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE 128

typedef struct {
    bool success;
    uint8_t value;
} read_result_t;

// cada instancia tem seu proprio array e seus proprios indices, entao dá
// para ter um buffer de recepcao e outro de transmissao sem que um saiba
// do outro. volatile porque um dos lados costuma ser a interrupcao.
typedef struct {
    uint8_t data[BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} ring_buffer_t;

bool ring_buffer_put(ring_buffer_t *buffer, uint8_t byte);
read_result_t ring_buffer_get(ring_buffer_t *buffer);

#endif /* INC_LOGIC_RING_BUFFER_H */
