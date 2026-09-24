#include "core.h"

bool config_validate(config_t raw_config) {
  return raw_config.magic == CONFIG_SIGNATURE;
}