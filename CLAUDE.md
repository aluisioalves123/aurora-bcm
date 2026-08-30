# Aurora BCM — STM32F446RE

Body control module automotivo escrito do zero, sem HAL, para um cliente
fictício (Aurora Implementos). **A especificação está em [DIRETRIZES.md](DIRETRIZES.md)
— leia antes de propor qualquer coisa**: 10 funções (F1–F10), 5 restrições
(R1–R5) e 13 etapas. Cada etapa entrega uma função do cliente.

Etapas 01 a 04 concluídas (setas, pisca-alerta, farol PWM com rampa). Próxima:
**05 — console de diagnóstico (F8)**, com recepção por interrupção, buffer
circular e parser byte a byte.

## Como trabalhar com o Aluisio neste projeto

**Modo proxy.** Ele diz o que quer, você escreve a sintaxe. Sem mudanças bruscas
sem ordem explícita. **Se ele estiver errado, deixe errado** — descobrir o erro
na placa faz parte do aprendizado.

Exceção: organizar em pastas e arquivos é permitido, mas **avise antes e espere
autorização**, para ele acompanhar para onde as coisas vão.

Relatar fato continua valendo — compilou, tamanho do binário, warning, o que não
foi testado. Isso é resultado, não correção.

**Não instale nada sem perguntar.** Não existe toolchain ARM no PATH desta
máquina; tudo vem embutido no STM32CubeIDE 2.2.0.

## Restrição que decide arquitetura

**R4 — nenhuma função pode atrasar outra.** Proíbe espera bloqueante em qualquer
lugar. Toda funcionalidade é máquina de estados alimentada pela base de tempo de
1 ms do SysTick. Se uma solução proposta tem `delay`, ela está errada por
requisito, não por gosto.

## Convenções do projeto

**Todo módulo habilita o que usa, por completo: porta, pino e clock.** Mesmo que
repita um `rcc_periph_clock_enable` que outro módulo já fez. O motivo não é
esquecimento, é acoplamento invisível: se o `service_light` funciona por causa de
uma linha dentro do `buttons_setup()`, nada no código diz isso e a quebra aparece
meses depois, num commit que mexeu em botões.

**Núcleo puro, casca imperativa.** Só as funções que traduzem pino ↔ valor tocam
hardware (`read_buttons`, `blink_leds`, `activate_service_light`). Toda decisão é
função pura: recebe tudo por parâmetro, devolve o resultado, não lê nem escreve
global. O estado que sobrevive entre iterações mora à vista no `main`.

**Camadas:** `hal/` fala com hardware, `logic/` só decide, `board.h` é o único
lugar com pino e porta, `main.c` orquestra e não conhece pino nenhum.

## Mapa de pinos

Fonte de verdade: `app/inc/board.h`.

| Função | Pino | Observação |
|---|---|---|
| Seta direita (LED) | PA5 | LD2 da placa |
| Seta esquerda (LED) | PA6 | |
| Botão seta direita | PB6 | pull-up interno, ativo em baixo |
| Botão seta esquerda | PC7 | idem |
| Botão pisca-alerta | PA9 | idem |
| Botão luz de serviço | PC13 | B1 da placa, idem |
| Farol de trabalho (PWM) | PB10 | AF1, TIM2 canal 3 |

TIM2: `PSC = 224`, `ARR = 399` → 2 kHz a 180 MHz. `CCR3` anda um passo por
systick, então a rampa completa leva 400 ms (F3).

## Toolchain (caminhos reais)

Raiz: `C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\`

| Ferramenta | Subpasta do plugin |
|---|---|
| GCC 14.3.1 + gdb | `...externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin` |
| make 4.4.1 | `...externaltools.make.win32_2.2.200.202604021615\tools\bin` |
| OpenOCD 0.12 (ST) | `...externaltools.openocd.win32_2.4.500.202604080855\tools\bin` |
| Scripts do OpenOCD | `...debug.openocd_2.3.400.202606220929\resources\openocd\st_scripts` |
| SVD do F446 | `...productdb.debug_2.2.500.202605201745\resources\cmsis\STMicroelectronics_CMSIS_SVD\STM32F446.svd` |

Esses caminhos estão gravados em `.vscode/tasks.json`, `launch.json`,
`settings.json` e `c_cpp_properties.json`. **Se atualizar o CubeIDE, as versões
nos nomes das pastas mudam e os quatro arquivos quebram juntos.**

Pela linha de comando, os tasks usam **Git Bash**, não PowerShell:

```sh
export PATH="$CUBE_GCC:$CUBE_MAKE:$PATH"
make -C app                 # gera app/firmware.elf
```

## Armadilhas já resolvidas — não redescubra

1. **`python3` do PATH é o stub falso da Microsoft Store** (erro 9009). O Python
   real é o standalone do uv em
   `%USERPROFILE%\AppData\Roaming\uv\python\cpython-3.11-windows-x86_64-none\`,
   que só tem `python.exe`. Como o gerador de `nvic.h` do libopencm3 tem shebang
   `#!/usr/bin/env python3`, existe um shim em `tools/bin/python3` (gitignored).
   Dentro dele o caminho **precisa** ser estilo `C:/...` — o estilo `/c/...`
   falha quando invocado pelo make. Só necessário ao recompilar o libopencm3.

2. **O OpenOCD da ST rejeita `interface/stlink.cfg`** (HLA, erro em `swj_newdap`).
   Use `interface/stlink-dap.cfg` + `-c "transport select dapdirect_swd"`.

3. **`board/st_nucleo_f4.cfg` não existe** nos scripts da ST. Use
   `interface/stlink-dap.cfg` + `target/stm32f4x.cfg`.

4. **Aspas aninhadas quebram task do VS Code no Windows.** Comando com `"` dentro
   de string única some ao passar pelo `bash -c`. O task `flash` usa
   `type: process` com argumentos em array por causa disso.

5. **F401RE → F446RE:** a RAM do `linkerscript.ld` é 128K, não 96K. Confirmação
   na placa: o MSP inicial lê `0x20020000`.

## Projeto irmão

O curso [Bare Metal Programming Series](https://github.com/lowbyteproductions/bare-metal-series)
é acompanhado em repositório separado
(`../Bare Metal Programmimg Series`). As etapas 01 a 04 daqui nasceram lá. Os
dois divergiram: o curso segue para bootloader, este projeto para o console.
Consulte o outro repo quando uma etapa precisar de um recurso que o curso cobre
(bootloader → etapa 11, UART e ring buffer → etapa 05).
