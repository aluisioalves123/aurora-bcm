#ifndef HAL_SERVICE_LIGHT_H
#define HAL_SERVICE_LIGHT_H

#include "logic/service_light/core.h"

void service_light_setup(void);

// chamada a cada systick: com a luz ligada, sobe o CCR de 1 em 1 ate
// alcancar o ARR; com a luz desligada, zera o CCR
void activate_service_light(service_light_state_t service_light_state);

uint32_t service_light_level(void);

#endif // HAL_SERVICE_LIGHT_H
