#include "service.h"

bool save_config(config_t config) {
  config.magic = CONFIG_SIGNATURE;
  return config_storage_write(config);
}

config_t load_config(void) {
  config_t config_raw;
  config_storage_read(&config_raw);

  if(config_validate(config_raw)) {
    return config_raw;
  } else {
    return config_default;
  }
}