#include "driver.h"
#include <libopencm3/cm3/scb.h>

void hard_fault_handler(void) {
  scb_reset_system();
}