#include "logic/buttons.h"

debounce_counters_t next_debounce(debounce_counters_t counters,
                                  button_states_t raw) {
  return (debounce_counters_t){
    .turn_signal_right = raw.turn_signal_right_button_pressed ? counters.turn_signal_right + 1 : 0,
    .turn_signal_left  = raw.turn_signal_left_button_pressed  ? counters.turn_signal_left  + 1 : 0,
    .hazard            = raw.hazard_button_pressed            ? counters.hazard            + 1 : 0,
    .service_light     = raw.service_light_button_pressed     ? counters.service_light     + 1 : 0
  };
}

button_states_t debounced_buttons(debounce_counters_t counters) {
  return (button_states_t){
    .turn_signal_right_button_pressed = (counters.turn_signal_right > DEBOUNCE_TICKS),
    .turn_signal_left_button_pressed  = (counters.turn_signal_left  > DEBOUNCE_TICKS),
    .hazard_button_pressed            = (counters.hazard            > DEBOUNCE_TICKS),
    .service_light_button_pressed     = (counters.service_light     > DEBOUNCE_TICKS)
  };
}
