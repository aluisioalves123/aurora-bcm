# Aurora BCM — diretrizes do projeto

> Transcrição do artefato de especificação
> ([original](https://claude.ai/code/artifact/56e463b5-244a-4276-991a-4a1b604c296e)).
> Este arquivo é a fonte de verdade do que o projeto precisa entregar.

Um módulo de carroceria automotivo construído do zero, em treze etapas. Cada
etapa entrega uma função que o cliente pediu, e puxa um módulo da trilha porque
a função exige, não porque estava na lista.

| | |
|---|---|
| ECU | Nucleo-F446RE |
| Carro simulado | Arduino Mega |
| Etapas | 13 |
| Custo inicial | R$ 0 |

## O cliente

**Aurora Implementos** é uma encarroçadora do interior de São Paulo. Compra
chassi de montadora e constrói a carroceria por cima: ambulâncias, viaturas,
vans escolares, utilitários de serviço.

O problema dela é real e comum no setor: o módulo elétrico que vem de fábrica no
chassi cuida do que a montadora previu, e nada mais. Tudo que a Aurora
acrescenta — sinaleira de teto, faróis de trabalho, iluminação do compartimento,
travas do baú, sensor de porta traseira — hoje é ligado com relé e fusível
avulsos. O chicote vira uma bagunça, cada veículo sai um pouco diferente, e
quando dá defeito no cliente ninguém sabe dizer o que aconteceu.

Eles querem um módulo de carroceria próprio: uma peça só, com firmware, que
controla tudo que a Aurora acrescenta, conversa com o veículo pelo barramento
CAN, e sabe dizer o que deu errado.

> **Ficção com fundo verdadeiro.** A Aurora não existe, mas o problema sim.
> Encarroçadoras e fabricantes de implementos realmente desenvolvem módulos
> próprios, e é um nicho onde firmware embarcado é contratado no Brasil. O polo
> automotivo de Campinas e Hortolândia vive disso.

## A bancada

```
 Arduino Mega              Aurora BCM                  Cargas
 o veículo simulado  ──▶   Nucleo-F446RE      ──▶   LEDs na protoboard
 alavanca de seta          o que você               setas, faróis,
 sensor de porta           está construindo         luz de cortesia
 tensão de bateria                │                        │
 CAN (etapa 10)                   │ serial          corrente de volta
                                  ▼
                                 PC
                        console de diagnóstico
```

O Mega faz o papel do veículo: gera os sinais que na vida real viriam do chassi
e das chaves do painel. A Nucleo é a ECU sendo desenvolvida. As cargas são LEDs
numa protoboard, com o resistor dimensionado para você medir a corrente de
volta. É assim que se testa ECU na indústria, com o veículo substituído por um
simulador de sinais.

## Funções

| Ref | O que o cliente quer | Requisito verificável |
|---|---|---|
| **F1** | As setas têm que piscar como no carro | Frequência entre 60 e 120 ciclos por minuto, medida com analisador lógico. Começa aceso, não apagado. |
| **F2** | Pisca-alerta | Aciona as duas setas em fase. Tem prioridade sobre a seta individual e a sobrepõe enquanto ativo. |
| **F3** | Faróis de trabalho com brilho ajustável | Saída PWM com brilho de 0 a 100%, rampa de acendimento e apagamento de 400 ms, sem cintilar. |
| **F4** | Luz acende quando abre a porta | Entrada digital com filtro de ruído. Acende em rampa e apaga sozinha depois de um tempo configurável. |
| **F5** | Saber se uma lâmpada queimou | Mede a corrente da saída quando ela está ligada. Abaixo do limiar, registra falha e sinaliza. |
| **F6** | Avisar se a bateria estiver fraca | Mede a tensão de alimentação. Fora da faixa, registra e entra em modo de proteção. |
| **F7** | Conversar com o veículo | Recebe comandos e publica estado no barramento CAN, em mensagens periódicas. |
| **F8** | Um técnico precisa conseguir diagnosticar | Console serial com comandos de status, leitura de falhas e teste de saída. |
| **F9** | Configurar sem trocar a peça | Parâmetros guardados em memória não volátil, ajustáveis pelo console. |
| **F10** | Atualizar sem tirar do veículo | Bootloader que recebe firmware pela serial, valida integridade e volta à versão anterior se falhar. |

## Restrições

| Ref | Restrição | Por quê |
|---|---|---|
| **R1** | Da energia até a seta funcionar: menos de 200 ms | Ninguém aceita esperar o carro "bootar" para dar seta. É o requisito que proíbe Linux nesta peça. |
| **R2** | Nunca pode travar | Watchdog obrigatório. Se travar, reinicia sozinho e registra a causa do reset. |
| **R3** | Consumo em repouso muito baixo | O veículo pode ficar semanas parado. O módulo não pode descarregar a bateria. |
| **R4** | Nenhuma função pode atrasar outra | A seta não pode hesitar porque a rampa do farol está rodando. Nada de espera bloqueante. |
| **R5** | Toda falha fica registrada | O técnico da concessionária precisa ler o que aconteceu depois do fato. |

> **Repare no que a R4 já decidiu.** Ela proíbe delay bloqueante em qualquer
> lugar do firmware. Isso significa que, a partir da etapa 2, tudo vira máquina
> de estados. Não é preferência de estilo, é consequência direta de um requisito
> do cliente. É assim que decisão de arquitetura nasce na vida real.

## As treze etapas

Cada uma entrega uma função e deixa o produto funcionando. Nenhuma etapa termina
com algo pela metade.

### Fase 1 — o módulo nasce, sem comprar nada

**01. Uma seta que pisca**
A função mais simples do módulo, feita do jeito mais difícil: registrador puro,
sem HAL, com o RM0390 aberto.
*Entrega:* um LED piscando, controlado por código que você entende linha a linha.
*Puxa:* módulos 2 e 3 — toolchain, startup, GPIO no registrador.

**02. Piscar na frequência certa** · `F1`
O laço de espera do passo anterior não serve: a frequência depende do compilador
e da otimização. Suba o clock para 180 MHz, monte base de tempo de 1 ms com o
SysTick, conte milissegundos em vez de instruções.
*Entrega:* pisca em 1,5 Hz exato, medido no analisador lógico.
*Puxa:* módulo 4 — RCC, PLL, prescalers, latência de flash, SysTick.
*Aprende:* por que "funciona na minha máquina" não existe em firmware.

**03. A alavanca de seta e o pisca-alerta** · `F2` `R4`
Alavanca com três posições e botão de alerta que sobrepõe tudo: quatro estados
com regra de prioridade. Entrada por interrupção de pino, com filtro de ruído.
*Entrega:* setas e alerta com prioridade correta, comandados pelo Mega.
*Puxa:* módulo 5 (interrupções, EXTI, NVIC, seções críticas) e a primeira metade
do 12 (máquina de estados).
*Cuidado:* a flag que a interrupção seta e o laço principal lê é o caso clássico
de `volatile` e corrida. Provoque o bug de propósito antes de consertar.

**04. Faróis com brilho e rampa** · `F3`
Ligar um farol de estalo incomoda a vista e dá pico de corrente. Rampa de 400 ms
significa PWM com o valor de comparação subindo aos poucos. E a R4 morde: a
rampa não pode travar a seta.
*Entrega:* farol acendendo e apagando suavemente enquanto a seta pisca sem hesitar.
*Puxa:* módulo 6 — timers, PWM, cálculo de prescaler e período.
*Extra:* brilho linear não é percebido como linear. Corrigir isso impressiona em
portfólio.

### Fase 2 — o módulo passa a se explicar

**05. Console de diagnóstico** · `F8`
Console serial com comandos: `status`, `saida 3 on`, `brilho 60`, `falhas`.
Recepção por interrupção com buffer circular, e um parser que monta o comando
byte a byte sem travar o resto.
*Entrega:* console funcionando pela USB da Nucleo, sem hardware extra.
*Puxa:* módulo 7 inteiro. Reaproveita os exercícios 11 e 12 do caderno de C.
*Ponte:* é a etapa que amarra o caderno de exercícios ao projeto. O que você
escreveu no PC vira firmware de verdade.

**06. Medir a bateria e detectar lâmpada queimada** · `F5` `F6`
Tensão de alimentação por divisor resistivo, corrente de saída por shunt. Se a
saída está ligada mas quase não passa corrente, o filamento abriu — diagnóstico
de verdade, do tipo que existe em carro de produção.
*Entrega:* console reportando tensão em volts e o estado de cada saída, incluindo
"lâmpada aberta".
*Puxa:* módulos 9 e 10 — ADC, tempo de amostragem, DMA circular disparado por
timer, filtragem por média.
*Custo:* resistores e um potenciômetro, uns R$ 15.

**07. Falhas registradas** · `R5`
Não basta detectar, tem que guardar. Tabela de códigos de falha com contador de
ocorrências e estado atual, no espírito dos códigos de diagnóstico que todo carro
tem. O console ganha os comandos de ler e limpar falhas.
*Entrega:* provoque uma falha desconectando um LED, veja o código aparecer, limpe
pelo console.
*Puxa:* arquitetura do módulo 12, agora com camadas: driver, serviço de
diagnóstico, aplicação.

### Fase 3 — o módulo vira produto

**08. Sobreviver ao mundo real** · `R2` `R3`
Watchdog alimentado do lugar certo, causa do último reset registrada, e modo de
baixo consumo quando o veículo está parado, acordando por porta aberta ou por
atividade no barramento. Junto: um tratador de falha que despeja o contexto pelo
console quando o firmware quebra.
*Entrega:* trave o firmware de propósito num laço infinito e veja o módulo
reiniciar sozinho, dizendo por quê.
*Puxa:* módulos 11 e 14 — análise de HardFault, IWDG, modos de energia, medição
de corrente de repouso.
*Vale ouro:* contar em entrevista que você provocou um fault e leu o registrador
de status para achar a linha culpada vale mais que qualquer lista de tecnologias.

**09. Configuração que sobrevive ao desligamento** · `F9`
Tempo da luz de cortesia, brilho padrão do farol, limiar de bateria baixa.
Gravar na flash em tempo de execução, com cuidado: apagar setor é lento e a flash
tem desgaste.
*Entrega:* mude o brilho pelo console, tire a energia, ligue de novo, o valor
continua lá.
*Puxa:* primeira metade do módulo 15 — organização de setores, apagar e gravar,
CRC de validação.

**10. Entrar no barramento CAN** · `F7`
O momento que motivou o projeto. O módulo publica seu estado periodicamente e
aceita comandos, com identificadores e filtros configurados. O Arduino Mega vira
o segundo nó, fazendo o papel do resto do veículo.
*Entrega:* comandar a seta pelo CAN, a partir do Mega, e ver o estado do módulo
aparecer do outro lado.
*Puxa:* ramo A da trilha — arbitragem, identificadores, filtros de aceitação,
tratamento de erro, estado bus-off.
*Custo:* dois transceptores CAN em módulo, ~R$ 50 o par. É a compra mais
importante do projeto.
*Empregabilidade:* é a etapa que faz o recrutador de Campinas parar no currículo.

**11. Atualizar sem tirar do veículo** · `F10`
Bootloader próprio: recebe a imagem nova pela serial, valida o CRC, grava, e
salta para a aplicação reposicionando a tabela de vetores. Se a imagem nova não
confirmar que subiu, volta para a anterior.
*Entrega:* grave uma versão nova pela serial e veja o módulo subir com ela.
Depois grave uma imagem corrompida de propósito e veja ele recusar e voltar.
*Puxa:* segunda metade do módulo 15 — layout de memória, VTOR, esquema A/B,
rollback.
*Raro:* poucos candidatos têm bootloader próprio no GitHub.

**12. Testes e integração contínua**
A lógica de prioridade das setas, o parser de comandos, a tabela de falhas e o
cálculo de tensão não precisam de hardware para serem testados. Separe do chip e
rode no PC. Depois, um pipeline que compila o firmware e roda os testes a cada
commit.
*Entrega:* repositório com badge de build verde e uma suíte que roda em segundos.
*Puxa:* módulo 16 — Unity ou CppUTest, dublês de driver, análise estática, CI.

**13. A placa da Aurora**
O produto final: PCB própria com o STM32F446, entradas protegidas, saídas com
driver de potência, transceptor CAN embarcado, conector automotivo, e um modo de
teste de fábrica que valida a placa recém-montada.
*Entrega:* do KiCad ao bring-up, com o firmware que você já tem rodando.
*Puxa:* módulo 17 — esquemático, layout, ordem de energização, desacoplamento,
separação de analógico e digital.
*Diferencial:* entregar hardware e firmware do mesmo projeto é raro em portfólio.

## O que comprar, e quando

| Etapa | O que precisa | Ordem de grandeza |
|---|---|---|
| 01 a 05 | Nada. Nucleo, Mega e a USB bastam | R$ 0 |
| 03 a 06 | Protoboard, jumpers, LEDs, resistores, potenciômetro | R$ 30 a 50 |
| 06 | Analisador lógico de 8 canais, se ainda não tiver | R$ 60 a 120 |
| 10 | Dois módulos transceptores CAN | R$ 50 a 80 |
| 13 | Fabricação da PCB, com frete | R$ 150 a 300 |

Estimativas de ordem de grandeza, não cotação. O gasto se dilui: você chega até
a etapa 5 sem comprar nada, e a maior despesa fica para o fim, quando já vai
saber se quer mesmo continuar.

## Ressalva honesta sobre o C

As etapas 1 e 2 vão bem com C enferrujado. A etapa 3 já pede ponteiro,
`volatile` e struct com desenvoltura. Da etapa 5 em diante, sem buffer circular e
máquina de estados na ponta dos dedos, você passa mais tempo brigando com a
linguagem do que com o problema.

A boa notícia é que os dois se alimentam: o exercício 11 do caderno de C é o
buffer do console da etapa 5, e o 12 é o parser dela. Você não está estudando C
para depois fazer o projeto — está escrevendo, no PC, as peças que vão entrar no
firmware.

---

*Aurora Implementos é um cliente fictício, criado para dar contexto ao
aprendizado. O produto, os requisitos e as restrições são plausíveis e refletem o
que módulos de carroceria reais fazem.*
