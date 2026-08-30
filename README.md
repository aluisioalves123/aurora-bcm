# Aurora BCM

Um módulo de carroceria automotivo (*body control module*) escrito do zero para
um **STM32F446RE**, sem HAL e sem código gerado — só registradores, linker script
e libopencm3 como camada fina.

O cliente é fictício: **Aurora Implementos**, uma encarroçadora que precisa de um
módulo próprio para tudo que ela acrescenta ao chassi — setas, faróis de
trabalho, luz de compartimento, travas, sensores de porta. O problema, esse, é
real: encarroçadoras desenvolvem módulos assim, e é um nicho onde firmware
embarcado é contratado no Brasil.

A especificação completa — 10 funções, 5 restrições e as 13 etapas — está em
**[DIRETRIZES.md](DIRETRIZES.md)**.

## A restrição que define a arquitetura

> **R4** — Nenhuma função pode atrasar outra. A seta não pode hesitar porque a
> rampa do farol está rodando. Nada de espera bloqueante.

Isso proíbe `delay` em qualquer lugar do firmware, e é o motivo de tudo aqui ser
máquina de estados alimentada por uma base de tempo comum. Não é preferência de
estilo — é consequência direta de um requisito.

## Progresso

| # | Etapa | Funções | Status |
|---|---|---|:---:|
| 01 | Uma seta que pisca | — | ✅ |
| 02 | Piscar na frequência certa | `F1` | ✅ |
| 03 | Alavanca de seta e pisca-alerta | `F2` `R4` | ✅ |
| 04 | Faróis com brilho e rampa | `F3` | ✅ |
| 05 | Console de diagnóstico | `F8` | ⬜ |
| 06 | Medir bateria e lâmpada queimada | `F5` `F6` | ⬜ |
| 07 | Falhas registradas | `R5` | ⬜ |
| 08 | Sobreviver ao mundo real | `R2` `R3` | ⬜ |
| 09 | Configuração não volátil | `F9` | ⬜ |
| 10 | Entrar no barramento CAN | `F7` | ⬜ |
| 11 | Atualizar sem tirar do veículo | `F10` | ⬜ |
| 12 | Testes e integração contínua | — | ⬜ |
| 13 | A placa da Aurora | — | ⬜ |

Falta ainda, das etapas já entregues: a alavanca de três posições ainda é botão,
a entrada é por varredura e não por interrupção `EXTI`, e a frequência do pisca
não foi medida com analisador lógico — só conferida a olho.

## Hardware e ferramentas

| | |
|---|---|
| ECU | NUCLEO-F446RE (Cortex-M4F, 512K flash / 128K RAM) |
| Veículo simulado | Arduino Mega *(a partir da etapa 03)* |
| Gravador | ST-Link V2-1 on-board, via SWD |
| Compilador | `arm-none-eabi-gcc` 14.3.1 |
| Biblioteca | [libopencm3](https://github.com/libopencm3/libopencm3) (submódulo) |
| Gravação/debug | OpenOCD 0.12 + Cortex-Debug no VS Code |

Todo o toolchain vem embutido no STM32CubeIDE — nada instalado à parte. Os
caminhos estão no [CLAUDE.md](CLAUDE.md).

## Estrutura

```
app/
├── inc/
│   ├── board.h              mapa do hardware: o único arquivo com pinos
│   ├── hal/                 systick  buttons  lamps  service_light
│   └── logic/               turn_signal  service_light
└── src/
    ├── main.c               a casca: setup e o laço
    ├── hal/                 fala com o hardware
    └── logic/               só decide, funções puras
```

O `main.c` não inclui `board.h`: ele não sabe que existe PA5 nem pull-up. Quando
a etapa 13 trocar a fiação por uma PCB, só o `board.h` muda.

## Compilar e gravar

Pelo VS Code (`Ctrl+Shift+P` → *Tasks: Run Task*):

| Task | O que faz |
|---|---|
| `build` | compila `app/firmware.elf` (também no `Ctrl+Shift+B`) |
| `flash` | compila e grava via OpenOCD |
| `clean` | limpa os artefatos |
| `libopencm3: build` | recompila a biblioteca (só na primeira vez) |

`F5` compila, grava e entra em debug parado no `main`.

Clonando do zero:

```sh
git clone --recursive https://github.com/aluisioalves123/aurora-bcm.git
```

## Estado atual do firmware

Três botões e dois LEDs implementam as setas com prioridade, e um quarto botão
controla o farol de trabalho em PWM com rampa de 400 ms — as duas coisas
rodando ao mesmo tempo, cada uma na sua máquina de estados, sem uma atrasar a
outra.

| Estado | Esquerdo | Direito |
|---|---|---|
| `SIGNAL_OFF` | aceso | aceso |
| `SIGNAL_RIGHT` | aceso | pisca |
| `SIGNAL_LEFT` | pisca | aceso |
| `SIGNAL_HAZARD` | pisca | pisca |

O farol usa TIM2 canal 3 no PB10 (AF1), `PSC = 224` e `ARR = 399` — 2 kHz de PWM
a 180 MHz. A cada systick o `CCR3` anda um passo, então percorrer os 400 níveis
leva exatamente os 400 ms que a `F3` pede, nos dois sentidos.

## Relação com o curso

As etapas 01 a 04 foram construídas acompanhando a
[Bare Metal Programming Series](https://github.com/lowbyteproductions/bare-metal-series)
da Low Byte Productions, num
[repositório separado](https://github.com/aluisioalves123/bare-metal-stm32f446re).
A partir daqui os dois divergem: o curso segue para bootloader, este projeto
segue para o console de diagnóstico. O curso continua sendo consultado quando
uma etapa precisar de um recurso que ele cobre.
