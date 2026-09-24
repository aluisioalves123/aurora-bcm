#ifndef LOGIC_VEHICLE_BUS_H
#define LOGIC_VEHICLE_BUS_H

// Contrato com o veiculo. No CAN o identificador nao e endereco de destino,
// e sim o assunto da mensagem. O outro lado deste acordo e o firmware em
// vehicle/src/main.cpp: mudar um numero aqui obriga a mudar la junto.
#define TURN_SIGNAL_LEFT_MESSAGE_ID   (0x100)
#define TURN_SIGNAL_RIGHT_MESSAGE_ID  (0x101)
#define HAZARD_MESSAGE_ID             (0x102)

#endif // LOGIC_VEHICLE_BUS_H
