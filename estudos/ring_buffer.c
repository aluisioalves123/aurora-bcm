/*
 * ring_buffer.c — implementação do buffer circular.
 *
 * Autoria: Aluisio Cunha. Escrito à mão, linha a linha, como estudo
 * da estrutura que vai virar a recepção da UART na etapa 05 do Aurora BCM.
 *
 * Nenhuma linha de lógica deste arquivo foi escrita por IA. O assistente
 * atuou só como compilador de plantão: compilava, rodava e relatava o
 * que o gcc dizia e o que o programa imprimia.
 *
 * Os testes estão em test_ring_buffer.c, e foram escritos por IA.
 */

#include "ring_buffer.h"

bool ring_buffer_put(ring_buffer_t *buffer, uint8_t byte) {
    if ((buffer->head + 1) % BUFFER_SIZE == buffer->tail) {
        return false;
    } else {
        buffer->data[buffer->head] = byte;
        if ((buffer->head + 1) % BUFFER_SIZE == 0) {
            buffer->head = 0;
        } else {
            buffer->head++;
        }

        return true;
    }
}

read_result_t ring_buffer_get(ring_buffer_t *buffer) {
    read_result_t read = {
        .success = false,
        .value = 0,
    };
    if (buffer->head != buffer->tail) {
        read.value = buffer->data[buffer->tail];
        read.success = true;
        if ((buffer->tail + 1) % BUFFER_SIZE == 0 && buffer->tail != 0) {
            buffer->tail = 0;
        } else {
            buffer->tail++;
        }
    }

    return read;
}
