#include "adc_hal.h"
#include "display_hal.h"
#include "system_state.h"
#include "gpio_hal.h"

static const uint32_t CELL_READ_FREQUENCY_MS = 250;


void setup() {
  hw_state_t state = HW_UNINITIALISED;

  Serial.begin(9600);
  while(!Serial){
    delay(1);
  }
  Serial.println("Initialising");


  if(!system_state_init()){
    state = HW_STATE_FAILED;
  } else if (!gpio_init()){
    state = HW_GPIO_INIT_FAILED;
  } else {
    state = HW_OK;
  }

  switch (state) {
    case HW_UNINITIALISED:
      system_state_set_loop_state(STATE_UNINITIALISED);
      break;
    case HW_OK:
      system_state_set_loop_state(STATE_START_UP);
      break;
    case HW_GPIO_INIT_FAILED:
    case HW_STATE_FAILED:
      system_state_set_loop_state(STATE_FAILED_SAFE);
      break;
    default:
      system_state_set_loop_state(STATE_FAILED_SAFE);
      break;
  }
  
}

void loop() {
  loop_state_t state;
  if(system_state_get_loop_state(&state) != SYSTEM_OK){
    // Advance state to failure mode
  }
  uint32_t now = millis();

  switch (state) {
    case STATE_START_UP:
      fsm_handler_start_up();
      break;
    case STATE_READ_CELL:
      fsm_handler_read_cell(now);
      break;
    case STATE_ADC_FAILURE:
      break;
    case STATE_UNINITIALISED:
      break;
    case STATE_FAILED_SAFE:
      break;
    default:
      system_state_set_loop_state(STATE_FAILED_SAFE);
      break;
  }
}

// 1. STATE MACHINE HANDLERS

void fsm_handler_start_up(void){
  Serial.println("Start up");
  delay(1000);
  system_state_set_loop_state(STATE_READ_CELL);
}

void fsm_handler_read_cell(const uint32_t now){
  uint32_t last_check_time = 0U;

  if(system_state_get_cell_check_time(&last_check_time) != SYSTEM_OK){
    system_state_set_loop_state(STATE_FAILED_SAFE);
    return;
  }
  if(has_timer_elapsed(now, last_check_time, CELL_READ_FREQUENCY_MS)){
    uint16_t calibration_value = 0;

    if(!gpio_read_calibration_pin(&calibration_value)){
      system_state_set_loop_state(STATE_FAILED_SAFE);
      return;
    }
    Serial.println(calibration_value);
    if(system_state_set_cell_check_time(now) != SYSTEM_OK){
      system_state_set_loop_state(STATE_FAILED_SAFE);
      return;
    }
  }
}

void fsm_handler_adc_failure(void){

}

void fsm_handler_failed_safe(void){
  Serial.println("FAILED SAFE!");
  delay(1000);
}


bool has_timer_elapsed(const uint32_t current_time, const uint32_t last_time, const uint32_t frequency){
  return (current_time - last_time) >= frequency; // See Note 9
}