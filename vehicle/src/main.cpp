// Veiculo simulado: tres botoes que publicam no barramento CAN.
//
// Em MCP_NORMAL o quadro sai no fio de verdade, e nao mais so dentro do
// controlador como no loopback que serviu para provar o SPI.
//
// CAN exige que outro no confirme cada quadro com um bit de ACK. Enquanto
// a Nucleo nao estiver recebendo, esse ACK nao vem: o envio aparece como
// FALHOU no serial e o contador de erro de transmissao sobe. E sintoma
// esperado de barramento com um no so, nao defeito de fiacao.

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

// ATENCAO: confira o numero gravado no cristal prateado do modulo, ao lado
// do MCP2515. Se for 16.000, troque para MCP_16MHZ. Com o cristal errado a
// taxa sai pela metade ou pelo dobro e os nos nao se entendem, sem nenhum
// erro aparecer.
#define CAN_CRISTAL MCP_8MHZ

static const uint8_t CAN_CS_PIN  = 10;
static const uint8_t CAN_INT_PIN = 2;

static const uint8_t BOTOES[] = { 4, 5, 6 };
static const uint8_t QUANTOS = sizeof(BOTOES) / sizeof(BOTOES[0]);
static const uint32_t DEBOUNCE_MS = 30;

// um identificador por botao. no CAN o ID nao e endereco de destino, e sim
// o assunto da mensagem — e tambem a prioridade: quanto menor, mais manda
static const uint16_t IDS[] = { 0x100, 0x101, 0x102 };

static MCP_CAN can(CAN_CS_PIN);

static bool anterior[QUANTOS];
static uint32_t ultima_mudanca[QUANTOS];

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < QUANTOS; i++) {
    pinMode(BOTOES[i], INPUT_PULLUP);
    anterior[i] = HIGH;
    ultima_mudanca[i] = 0;
  }

  pinMode(CAN_INT_PIN, INPUT);

  Serial.println();
  Serial.println("Aurora BCM - veiculo simulado, teste do MCP2515");

  if (can.begin(MCP_ANY, CAN_500KBPS, CAN_CRISTAL) != CAN_OK) {
    Serial.println("MCP2515: begin FALHOU - confira SPI, CS e alimentacao");
    return;
  }

  Serial.println("MCP2515: begin ok");

  can.setMode(MCP_NORMAL);
  Serial.println("modo: normal (o quadro vai para o fio)");
  Serial.println("aperte D4, D5 ou D6 para enviar");
}

static void envia(uint8_t indice) {
  uint8_t dados[1] = { indice };

  // (id, extendido? 0 = padrao de 11 bits, tamanho, dados)
  uint8_t resultado = can.sendMsgBuf(IDS[indice], 0, sizeof(dados), dados);

  Serial.print("enviado ID 0x");
  Serial.print(IDS[indice], HEX);
  Serial.println(resultado == CAN_OK ? " ok" : " FALHOU");
}

static void recebe(void) {
  unsigned long id = 0;
  uint8_t tamanho = 0;
  uint8_t dados[8];

  if (can.readMsgBuf(&id, &tamanho, dados) != CAN_OK) {
    return;
  }

  Serial.print("recebido ID 0x");
  Serial.print(id, HEX);
  Serial.print(" [");
  Serial.print(tamanho);
  Serial.print("]");

  for (uint8_t i = 0; i < tamanho; i++) {
    Serial.print(" ");
    Serial.print(dados[i], HEX);
  }

  Serial.println();
}

void loop() {
  uint32_t agora = millis();

  for (uint8_t i = 0; i < QUANTOS; i++) {
    bool nivel = digitalRead(BOTOES[i]);

    if (nivel == anterior[i]) {
      continue;
    }

    if (agora - ultima_mudanca[i] < DEBOUNCE_MS) {
      continue;
    }

    ultima_mudanca[i] = agora;
    anterior[i] = nivel;

    if (nivel == LOW) {
      envia(i);
    }
  }

  // o INT do MCP2515 e ativo em baixo: nivel baixo significa que ha quadro
  // esperando para ser lido
  if (digitalRead(CAN_INT_PIN) == LOW) {
    recebe();
  }
}
