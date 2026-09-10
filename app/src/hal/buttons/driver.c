#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "board.h"
#include "hal/buttons/driver.h"

void buttons_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);

  gpio_mode_setup(
    TURN_SIGNAL_RIGHT_BUTTON_PORT,
    GPIO_MODE_INPUT,
    GPIO_PUPD_PULLUP,
    TURN_SIGNAL_RIGHT_BUTTON_PIN
  );

  gpio_mode_setup(
    TURN_SIGNAL_LEFT_BUTTON_PORT,
    GPIO_MODE_INPUT,
    GPIO_PUPD_PULLUP,
    TURN_SIGNAL_LEFT_BUTTON_PIN
  );

  gpio_mode_setup(
    HAZARD_BUTTON_PORT,
    GPIO_MODE_INPUT,
    GPIO_PUPD_PULLUP,
    HAZARD_BUTTON_PIN
  );

  gpio_mode_setup(
    SERVICE_LIGHT_BUTTON_PORT,
    GPIO_MODE_INPUT,
    GPIO_PUPD_PULLUP,
    SERVICE_LIGHT_BUTTON_PIN
  );
}

button_states_t read_buttons(void) {
  return (button_states_t){
    .turn_signal_right_button_pressed = (gpio_get(TURN_SIGNAL_RIGHT_BUTTON_PORT, TURN_SIGNAL_RIGHT_BUTTON_PIN) == 0),
    .turn_signal_left_button_pressed  = (gpio_get(TURN_SIGNAL_LEFT_BUTTON_PORT, TURN_SIGNAL_LEFT_BUTTON_PIN) == 0),
    .hazard_button_pressed            = (gpio_get(HAZARD_BUTTON_PORT, HAZARD_BUTTON_PIN) == 0),
    .service_light_button_pressed     = (gpio_get(SERVICE_LIGHT_BUTTON_PORT, SERVICE_LIGHT_BUTTON_PIN) == 0)
  };
}

