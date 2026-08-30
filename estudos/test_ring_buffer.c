/*
 * test_ring_buffer.c — testes do buffer circular.
 *
 * Autoria: escrito por IA (Claude), a pedido de Aluisio Cunha.
 * A implementação testada está em ring_buffer.c e é de autoria dele.
 *
 * Roda no PC, não no alvo. Devolve 0 se tudo passa e 1 se algo falha,
 * que é o que um sistema de integração contínua lê.
 *
 *   gcc -Wall -Wextra -std=c11 -o testes.exe test_ring_buffer.c ring_buffer.c
 *   ./testes.exe
 */

#include <stdio.h>

#include "ring_buffer.h"

static int falhas = 0;

static void verifica(const char *nome, bool condicao)
{
    printf("[%s] %s\n", condicao ? " ok  " : "FALHA", nome);
    if (!condicao) {
        falhas++;
    }
}

/* os testes zeram head e tail na mão antes de cada cenário,
   para um não herdar o estado do outro */

int main(void) {
    ring_buffer_t rb = { .data = { 0 }, .head = 0, .tail = 0 };
    read_result_t r;
    bool ok;
    int i, entraram;

    /* 1. buffer recém-criado */
    rb.head = 0; rb.tail = 0;
    r = ring_buffer_get(&rb);
    verifica("ler de buffer vazio recusa", r.success == false);
    verifica("ler de buffer vazio nao mexe no tail", rb.tail == 0);

    /* 2. um byte entra e sai igual */
    rb.head = 0; rb.tail = 0;
    verifica("inserir em buffer vazio aceita", ring_buffer_put(&rb, 'a') == true);
    r = ring_buffer_get(&rb);
    verifica("leu de volta o byte inserido", r.success && r.value == 'a');
    r = ring_buffer_get(&rb);
    verifica("segunda leitura diz vazio", r.success == false);

    /* 3. capacidade: array de 8, cabem 7 */
    rb.head = 0; rb.tail = 0;
    entraram = 0;
    for (i = 0; i < BUFFER_SIZE + 4; i++) {
        if (ring_buffer_put(&rb, (uint8_t)('a' + i))) {
            entraram++;
        }
    }
    verifica("cabem exatamente BUFFER_SIZE-1 bytes", entraram == BUFFER_SIZE - 1);
    verifica("insercao em buffer cheio recusa", ring_buffer_put(&rb, 'z') == false);

    /* 4. sai na mesma ordem que entrou (o buffer do cenário 3 ainda está cheio) */
    ok = true;
    for (i = 0; i < BUFFER_SIZE - 1; i++) {
        r = ring_buffer_get(&rb);
        if (!r.success || r.value != (uint8_t)('a' + i)) {
            ok = false;
        }
    }
    verifica("saiu na ordem em que entrou (FIFO)", ok);
    verifica("vazio depois de tirar tudo", ring_buffer_get(&rb).success == false);

    /* 5. dar a volta no array várias vezes */
    rb.head = 0; rb.tail = 0;
    ok = true;
    for (i = 0; i < 30; i++) {
        uint8_t esperado = (uint8_t)('A' + (i % 26));

        if (!ring_buffer_put(&rb, esperado)) {
            ok = false;
            break;
        }
        r = ring_buffer_get(&rb);
        if (!r.success || r.value != esperado) {
            ok = false;
            break;
        }
    }
    verifica("30 idas e voltas seguidas", ok);
    verifica("indices seguem dentro do array", rb.head < BUFFER_SIZE && rb.tail < BUFFER_SIZE);

    /* 6. encher, liberar 3 casas, encher de novo */
    rb.head = 0; rb.tail = 0;
    while (ring_buffer_put(&rb, 'x')) {
    }
    for (i = 0; i < 3; i++) {
        ring_buffer_get(&rb);
    }
    entraram = 0;
    while (ring_buffer_put(&rb, 'y')) {
        entraram++;
    }
    verifica("apos liberar 3 casas, cabem 3 de novo", entraram == 3);

    /* 7. o byte 0 é dado válido — é o motivo de a struct ser melhor que sentinela */
    rb.head = 0; rb.tail = 0;
    ring_buffer_put(&rb, 0);
    r = ring_buffer_get(&rb);
    verifica("byte 0 e dado, nao sinal de vazio", r.success == true && r.value == 0);

    /* 8. o byte mais alto que cabe em uint8_t */
    rb.head = 0; rb.tail = 0;
    ring_buffer_put(&rb, 255);
    r = ring_buffer_get(&rb);
    verifica("byte 255 volta intacto", r.success && r.value == 255);

    printf("\n%d falha(s)\n", falhas);
    return falhas == 0 ? 0 : 1;
}
