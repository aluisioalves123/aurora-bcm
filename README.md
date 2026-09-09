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
| 05 | Console de diagnóstico | `F8` | ✅ |
| 06 | Medir bateria e lâmpada queimada | `F5` `F6` | 🟡 |
| 07 | Falhas registradas | `R5` | ⬜ |
| 08 | Sobreviver ao mundo real | `R2` `R3` | ⬜ |
| 09 | Configuração não volátil | `F9` | ⬜ |
| 10 | Entrar no barramento CAN | `F7` | ⬜ |
| 11 | Atualizar sem tirar do veículo | `F10` | ⬜ |
| 12 | Testes e integração contínua | — | ⬜ |
| 13 | A placa da Aurora | — | ⬜ |

🟡 = em andamento. Da etapa 06 já existem a leitura do ADC e a conversão para
volts e miliampères; falta o diagnóstico de lâmpada aberta e o modo de proteção
por bateria fraca.

Falta ainda, das etapas já entregues: a alavanca de três posições ainda é botão,
a entrada é por varredura e não por interrupção `EXTI`, a frequência do pisca não
foi medida com analisador lógico — só conferida a olho — e a leitura do ADC ainda
espera o fim da conversão num laço, que é justamente o tipo de espera que a `R4`
proíbe.

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
│   ├── version.h            versão do firmware, reportada pelo console
│   ├── app/                 terminal
│   ├── hal/                 systick  buttons  lamps  service_light  uart  adc
│   └── logic/               turn_signal  service_light  buttons  ring_buffer
│                            message  battery_millivolts  shunt_current
└── src/
    ├── main.c               a casca: setup e o laço
    ├── app/                 decide o que fazer com um comando já montado
    ├── hal/                 fala com o hardware
    └── logic/               só decide, funções puras

estudos/                     peças de C escritas no PC antes de virarem firmware
├── ring_buffer.c            o buffer circular, escrito à mão
└── test_ring_buffer.c       14 asserções, rodam no PC, exit 1 se alguma falha
```

`logic/` não conhece hardware — nem por header. Quem lê o pino é o `hal/`, quem
decide é o `logic/`, e o `main.c` liga os dois: `next_debounce(debounce,
read_buttons())`. A camada `app/` fica acima das duas e cuida do que é política
de produto, como a tabela de comandos do console.

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

O farol usa TIM2 canal 3 no PB10 (AF1), `PSC = 224` e `ARR = 399` — 1 kHz de PWM
a 180 MHz. A cada systick o `CCR3` anda um passo, então percorrer os 400 níveis
leva exatamente os 400 ms que a `F3` pede, nos dois sentidos.

### Console de diagnóstico

Pela mesma USB da gravação, a 115200 8N1, sem hardware extra. A recepção é por
interrupção: cada byte cai num buffer circular, o laço principal tira um por vez
e monta a frase, e a mensagem só sobe quando chega `\r` ou `\n` — nada disso
espera por nada. A transmissão também é por interrupção: `print_serial`
enfileira e volta na hora, independente do tamanho do texto.

| Comando | O que responde |
|---|---|
| `/help` | a lista de comandos |
| `/hello` | `hello world` |
| `/status` | versão, uptime, estado da seta, nível do farol, bytes perdidos no RX e no TX |
| `/adc_val` | leitura crua do canal do shunt |
| `/battery_val` | tensão da bateria, em volts |
| `/shunt_current` | corrente pelo shunt, em miliampères |

Byte que chega com o buffer cheio não some calado: vira contador, e o `/status`
mostra. Perder pode acontecer; perder em silêncio, não.

O buffer circular foi escrito à mão no PC, antes de entrar no firmware, com uma
suíte de 14 asserções em `estudos/` que roda em segundos e devolve código de
saída — o formato que a etapa 12 vai pedir da integração contínua.

## Relação com o curso

As etapas 01 a 04 foram construídas acompanhando a
[Bare Metal Programming Series](https://github.com/lowbyteproductions/bare-metal-series)
da Low Byte Productions, num
[repositório separado](https://github.com/aluisioalves123/bare-metal-stm32f446re).
A partir daqui os dois divergem: o curso segue para bootloader, este projeto
segue para o console de diagnóstico. O curso continua sendo consultado quando
uma etapa precisar de um recurso que ele cobre.
