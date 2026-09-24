#include <libopencm3/cm3/scb.h>
#include <stdint.h>

#define APPLICATION_ADDRESS 0x08004000

int main(void) {
  uint32_t app_stack_pointer  = *(volatile uint32_t *)APPLICATION_ADDRESS;
  uint32_t app_reset_handler  = *(volatile uint32_t *)(APPLICATION_ADDRESS + 4);

  SCB_VTOR = APPLICATION_ADDRESS;

  __asm__ volatile ("msr msp, %0" : : "r" (app_stack_pointer));

  void (*app_reset)(void) = (void (*)(void)) app_reset_handler;
  app_reset();
  
  while(1);
}