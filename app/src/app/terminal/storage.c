// Comandos que mexem no cartao.
//
// Os dois moram no mesmo arquivo porque dividem o mesmo FIL: separa-los
// obrigaria a duplicar 560 bytes de buffer ou a promover essa estrutura a um
// header, e nenhuma das duas paga o preco.

#include "app/terminal/commands.h"

#include <stdio.h>

#include "ff.h"
#include "hal/uart/driver.h"

// O FIL e static so por causa da pilha: ele carrega um setor inteiro dentro,
// sao ~560 bytes. Ao contrario do FATFS, nao precisa sobreviver a funcao — o
// f_close encerra o uso dele aqui mesmo.
static FIL log_file;

void command_log(const char *argument, console_t *console) {
  (void)argument;
  (void)console;

  char linha[64];
  const char texto[] = "Aurora BCM - teste de escrita\r\n";
  UINT escritos = 0;

  // FA_OPEN_APPEND cria o arquivo se nao existir e posiciona no fim se existir,
  // que e o comportamento de log
  FRESULT abertura = f_open(&log_file, "log.txt", FA_WRITE | FA_OPEN_APPEND);

  snprintf(linha, sizeof(linha), "f_open: %s (codigo %d)\r\n",
      abertura == FR_OK ? "ok" : "falhou", (int)abertura);
  print_serial(linha);

  if (abertura != FR_OK) {
    return;
  }

  // sizeof - 1 para nao gravar o terminador: ele fecha a string na memoria,
  // nao faz parte do texto
  FRESULT escrita = f_write(&log_file, texto, sizeof(texto) - 1, &escritos);

  snprintf(linha, sizeof(linha), "f_write: %s (codigo %d, %u bytes)\r\n",
      escrita == FR_OK ? "ok" : "falhou", (int)escrita, (unsigned)escritos);
  print_serial(linha);

  // fecha mesmo se a escrita falhou: e o f_close que descarrega o buffer e
  // atualiza o tamanho do arquivo na tabela
  FRESULT fechamento = f_close(&log_file);

  snprintf(linha, sizeof(linha), "f_close: %s (codigo %d)\r\n",
      fechamento == FR_OK ? "ok" : "falhou", (int)fechamento);
  print_serial(linha);
}

// Le em pedacos de 512 e imprime conforme le. O teto existe porque a fila de
// transmissao tem tamanho fixo: sem ele, um log grande seria descartado pela
// metade e voce leria um arquivo truncado sem saber.
#define LOG_READ_MAX_BYTES 2048

static char log_chunk[513];

void command_log_read(const char *argument, console_t *console) {
  (void)argument;
  (void)console;

  char linha[64];
  UINT lidos = 0;
  uint32_t total = 0;

  FRESULT abertura = f_open(&log_file, "log.txt", FA_READ);

  snprintf(linha, sizeof(linha), "f_open: %s (codigo %d)\r\n",
      abertura == FR_OK ? "ok" : "falhou", (int)abertura);
  print_serial(linha);

  if (abertura != FR_OK) {
    return;
  }

  snprintf(linha, sizeof(linha), "tamanho: %lu bytes\r\n",
      (unsigned long)f_size(&log_file));
  print_serial(linha);
  print_serial("-- conteudo --\r\n");

  do {
    FRESULT leitura = f_read(&log_file, log_chunk, sizeof(log_chunk) - 1, &lidos);

    if (leitura != FR_OK) {
      snprintf(linha, sizeof(linha), "f_read: falhou (codigo %d)\r\n", (int)leitura);
      print_serial(linha);
      break;
    }

    // byte de controle que nao seja fim de linha vira ponto, senao um arquivo
    // com lixo dentro embaralha o terminal. o zero encerraria a string cedo.
    for (UINT i = 0; i < lidos; i++) {
      char byte = log_chunk[i];
      if (byte != 0x0D && byte != 0x0A && (byte < 0x20 || byte == 0x7F)) {
        log_chunk[i] = 0x2E;
      }
    }
    log_chunk[lidos] = 0;

    print_serial(log_chunk);
    total += lidos;
  } while (lidos == sizeof(log_chunk) - 1 && total < LOG_READ_MAX_BYTES);

  if (total < f_size(&log_file)) {
    snprintf(linha, sizeof(linha), "\r\n-- cortado em %lu de %lu bytes --\r\n",
        (unsigned long)total, (unsigned long)f_size(&log_file));
    print_serial(linha);
  }

  FRESULT fechamento = f_close(&log_file);

  snprintf(linha, sizeof(linha), "f_close: %s (codigo %d)\r\n",
      fechamento == FR_OK ? "ok" : "falhou", (int)fechamento);
  print_serial(linha);
}
