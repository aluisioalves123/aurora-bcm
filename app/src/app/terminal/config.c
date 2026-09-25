// Configuracao programavel: /config <nome> <valor>
//
//   /config temperature_limit 24.5      limiar de temperatura, em graus Celsius
//
// Grava na flash, entao o valor sobrevive a reset e a desligar a placa. Com a
// flash virgem ou a assinatura errada, o load_config devolve os padroes.
//
// Quando aparecer a segunda configuracao isto pede uma tabela de nome ->
// campo, do mesmo jeito que a tabela de comandos do service.c. Com uma so, a
// comparacao direta ainda e mais legivel do que a tabela.

#include "app/terminal/commands.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app/config/service.h"
#include "hal/uart/driver.h"

void command_config(const char *argument, console_t *console) {
  char linha[80];
  char nome[32];
  const char *p = argument;
  size_t i = 0;

  // o nome vai ate o primeiro espaco
  while (*p != 0x20 && *p != 0 && i < sizeof(nome) - 1) {
    nome[i++] = *p++;
  }
  nome[i] = 0;

  while (*p == 0x20) {
    p++;
  }

  if (nome[0] == 0 || *p == 0) {
    print_serial("uso: /config <nome> <valor>\r\n");
    print_serial("  temperature_limit  limiar de temperatura, em graus Celsius (ex: 24.5)\r\n");
    return;
  }

  // o tecnico manda em graus Celsius, com ponto decimal: 24.5. quem converte
  // para milicelsius e aqui, porque milicelsius e unidade de codigo, nao de
  // painel. nada abaixo desta funcao volta a ver grau
  bool negativo = (*p == 0x2D);
  if (negativo) {
    p++;
  }

  int32_t inteiro = 0;
  const char *digitos = p;

  while (*p >= 0x30 && *p <= 0x39) {
    inteiro = inteiro * 10 + (int32_t)(*p - 0x30);
    p++;

    // seis digitos ja passam de qualquer temperatura concebivel, e mais que
    // isso estouraria o int32 depois de multiplicar por mil
    if (p - digitos > 6) {
      print_serial("valor invalido: numero grande demais\r\n");
      return;
    }
  }

  if (p == digitos) {
    print_serial("valor invalido: esperado graus Celsius, ex: 24.5\r\n");
    return;
  }

  // a parte decimal vale ate tres casas, que e o que milicelsius comporta
  int32_t milesimos = 0;

  if (*p == 0x2E) {
    p++;

    if (!(*p >= 0x30 && *p <= 0x39)) {
      print_serial("valor invalido: ponto decimal sem digito depois\r\n");
      return;
    }

    int32_t casa = 100;

    while (*p >= 0x30 && *p <= 0x39) {
      if (casa == 0) {
        print_serial("valor invalido: no maximo tres casas decimais\r\n");
        return;
      }
      milesimos += (int32_t)(*p - 0x30) * casa;
      casa /= 10;
      p++;
    }
  }

  // lixo depois do numero: nao da para adivinhar o que o tecnico quis
  if (*p != 0) {
    print_serial("valor invalido: esperado graus Celsius, ex: 24.5\r\n");
    return;
  }

  int32_t valor = inteiro * 1000 + milesimos;

  if (negativo) {
    valor = -valor;
  }

  if (strcmp(nome, "temperature_limit") != 0) {
    print_serial("configuracao desconhecida\r\n");
    print_serial("  temperature_limit  limiar de temperatura, em graus Celsius (ex: 24.5)\r\n");
    return;
  }

  console->config->temperature_millicelsius_limit = valor;

  // so anuncia depois de a flash confirmar: dizer que gravou sem ter gravado
  // seria pior do que falhar
  if (!save_config(*console->config)) {
    print_serial("config: gravacao na flash falhou\r\n");
    return;
  }

  // devolve em grau, na mesma unidade que ele digitou: confirmacao em
  // milicelsius obrigaria o tecnico a conferir a conta de cabeca
  int32_t absoluto = (valor < 0) ? -valor : valor;

  snprintf(linha, sizeof(linha),
      "config: temperature_limit = %s%ld.%03ld C\r\n",
      (valor < 0) ? "-" : "",
      (long)(absoluto / 1000), (long)(absoluto % 1000));
  print_serial(linha);
}
