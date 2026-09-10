#include "driver.h"
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>

uint32_t read_and_clear_reset_cause_register(void) {
  uint32_t csr = RCC_CSR;
  RCC_CSR |= RCC_CSR_RMVF; // clear do registrador de reset cause
  return csr;
}

reset_cause_t reset_cause(uint32_t csr) {
  if (csr & RCC_CSR_LPWRRSTF) {
    return LOW_POWER;
  } else if (csr & RCC_CSR_WWDGRSTF) {
    return WINDOW_WATCHDOG;
  } else if (csr & RCC_CSR_IWDGRSTF) {
    return WATCHDOG;
  } else if (csr & RCC_CSR_SFTRSTF) {
    return SOFTWARE;
  } else if (csr & RCC_CSR_PORRSTF) {
    return POWER_ON;
  } else if (csr & RCC_CSR_PINRSTF) {
    return BUTTON_PRESS;
  } else if (csr & RCC_CSR_BORRSTF) {
    return BROWN_OUT;
  } else {
    return UNKNOWN;
  }
}

