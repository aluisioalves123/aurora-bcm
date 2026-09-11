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
    return RESET_LOW_POWER;
  } else if (csr & RCC_CSR_WWDGRSTF) {
    return RESET_WINDOW_WATCHDOG;
  } else if (csr & RCC_CSR_IWDGRSTF) {
    return RESET_WATCHDOG;
  } else if (csr & RCC_CSR_SFTRSTF) {
    return RESET_SOFTWARE;
  } else if (csr & RCC_CSR_PORRSTF) {
    return RESET_POWER_ON;
  } else if (csr & RCC_CSR_PINRSTF) {
    return RESET_BUTTON_PRESS;
  } else if (csr & RCC_CSR_BORRSTF) {
    return RESET_BROWN_OUT;
  } else {
    return RESET_UNKNOWN;
  }
}

