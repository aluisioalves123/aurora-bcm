#ifndef HAL_CONFIG_STORAGE_H
#define HAL_CONFIG_STORAGE_H

#include "logic/config/core.h"

bool config_storage_write(config_t config);

void config_storage_read(config_t *config);

#endif