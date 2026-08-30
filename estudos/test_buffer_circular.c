/*
 * test_buffer_circular.c — testes do buffer circular.
 *
 * Autoria: escrito por IA (Claude), a pedido de Aluisio Cunha.
 * A implementação testada está em buffer_circular.c e é de autoria dele.
 *
 * Roda no PC, não no alvo. Devolve 0 se tudo passa e 1 se algo falha,
 * que é o que um sistema de integração contínua lê.
 *
 *   gcc -Wall -Wextra -std=c11 -o testes.exe test_buffer_circular.c buffer_circular.c
 *   ./testes.exe
 */

#include <stdio.h>

#include "buffer_circular.h"

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
    resultado_leitura_t r;
    bool ok;
    int i, entraram;

    /* 1. buffer recém-criado */
    head = 0; tail = 0;
    r = ler_do_buffer();
    verifica("ler de buffer vazio recusa", r.success == false);
    verifica("ler de buffer vazio nao mexe no tail", tail == 0);

    /* 2. um byte entra e sai igual */
    head = 0; tail = 0;
    verifica("inserir em buffer vazio aceita", inserir_no_buffer('a') == true);
    r = ler_do_buffer();
    verifica("leu de volta o byte inserido", r.success && r.value == 'a');
    r = ler_do_buffer();
    verifica("segunda leitura diz vazio", r.success == false);

    /* 3. capacidade: array de 8, cabem 7 */
    head = 0; tail = 0;
    entraram = 0;
    for (i = 0; i < BUFFER_SIZE + 4; i++) {
        if (inserir_no_buffer((uint8_t)('a' + i))) {
            entraram++;
        }
    }
    verifica("cabem exatamente BUFFER_SIZE-1 bytes", entraram == BUFFER_SIZE - 1);
    verifica("insercao em buffer cheio recusa", inserir_no_buffer('z') == false);

    /* 4. sai na mesma ordem que entrou (o buffer do cenário 3 ainda está cheio) */
    ok = true;
    for (i = 0; i < BUFFER_SIZE - 1; i++) {
        r = ler_do_buffer();
        if (!r.success || r.value != (uint8_t)('a' + i)) {
            ok = false;
        }
    }
    verifica("saiu na ordem em que entrou (FIFO)", ok);
    verifica("vazio depois de tirar tudo", ler_do_buffer().success == false);

    /* 5. dar a volta no array várias vezes */
    head = 0; tail = 0;
    ok = true;
    for (i = 0; i < 30; i++) {
        uint8_t esperado = (uint8_t)('A' + (i % 26));

        if (!inserir_no_buffer(esperado)) {
            ok = false;
            break;
        }
        r = ler_do_buffer();
        if (!r.success || r.value != esperado) {
            ok = false;
            break;
        }
    }
    verifica("30 idas e voltas seguidas", ok);
    verifica("indices seguem dentro do array", head < BUFFER_SIZE && tail < BUFFER_SIZE);

    /* 6. encher, liberar 3 casas, encher de novo */
    head = 0; tail = 0;
    while (inserir_no_buffer('x')) {
    }
    for (i = 0; i < 3; i++) {
        ler_do_buffer();
    }
    entraram = 0;
    while (inserir_no_buffer('y')) {
        entraram++;
    }
    verifica("apos liberar 3 casas, cabem 3 de novo", entraram == 3);

    /* 7. o byte 0 é dado válido — é o motivo de a struct ser melhor que sentinela */
    head = 0; tail = 0;
    inserir_no_buffer(0);
    r = ler_do_buffer();
    verifica("byte 0 e dado, nao sinal de vazio", r.success == true && r.value == 0);

    /* 8. o byte mais alto que cabe em uint8_t */
    head = 0; tail = 0;
    inserir_no_buffer(255);
    r = ler_do_buffer();
    verifica("byte 255 volta intacto", r.success && r.value == 255);

    printf("\n%d falha(s)\n", falhas);
    return falhas == 0 ? 0 : 1;
}
