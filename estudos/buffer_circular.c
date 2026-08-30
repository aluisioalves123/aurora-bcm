/*
 * buffer_circular.c — implementação do buffer circular.
 *
 * Autoria: Aluisio Cunha. Escrito à mão, linha a linha, como estudo
 * da estrutura que vai virar a recepção da UART na etapa 05 do Aurora BCM.
 *
 * Nenhuma linha de lógica deste arquivo foi escrita por IA. O assistente
 * atuou só como compilador de plantão: compilava, rodava e relatava o
 * que o gcc dizia e o que o programa imprimia.
 *
 * Os testes estão em test_buffer_circular.c, e foram escritos por IA.
 */

#include "buffer_circular.h"

volatile uint32_t head = 0;
volatile uint32_t tail = 0;
static uint8_t buffer_circular[BUFFER_SIZE]; //array de chars de 8 posições

bool inserir_no_buffer(uint8_t letra) {
    if ((head + 1) % BUFFER_SIZE == tail) {
        return false;
    } else {
        buffer_circular[head] = letra;
        if ((head + 1) % BUFFER_SIZE == 0) {
            head = 0;
        } else {
            head++;
        }

        return true;
    }
}

resultado_leitura_t ler_do_buffer(void) {
    resultado_leitura_t read = {
        .success = false,
        .value = 0,
    };
    if (head != tail) {
        read.value = buffer_circular[tail];
        read.success = true;
        if ((tail + 1) % BUFFER_SIZE == 0 && tail != 0) {
            tail = 0;
        } else {
            tail++;
        }
    }

    return read;
}
