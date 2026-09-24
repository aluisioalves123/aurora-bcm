#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "hal/config_storage/driver.h"
#include "logic/config/core.h"

bool save_config(config_t config);

config_t load_config(void);

#endif