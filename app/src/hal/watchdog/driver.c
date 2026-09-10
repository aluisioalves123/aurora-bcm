#include "hal/watchdog/driver.h"
#include <libopencm3/stm32/iwdg.h>

void watchdog_setup(void) {
  iwdg_set_period_ms(10000);
  iwdg_start();
}

void watchdog_feed(void) {
  iwdg_reset();
}