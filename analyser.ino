#include "adc_hal.h"
#include "display_hal.h"
#include "system_state.h"
#include "gpio_hal.h"
#include <Wire.h>

static const uint32_t CELL_READ_FREQUENCY_MS = 250;
static const uint16_t MAX_CALIBRATION_READING_RAW = 750;

const uint16_t POT_MIN = 0U;
const uint16_t POT_MAX = 700U;

const uint16_t CAL_PERCENT_MIN = 75U;
const uint16_t CAL_PERCENT_MAX = 125U;

const uint16_t ADC_REFERENCE_RAW = 1600U;
const uint16_t ADC_REFERENCE_FO2_X10 = 210U; // 21.0%



void setup() {
  hw_state_t state = HW_UNINITIALISED;

  Wire.begin();

  Serial.begin(9600);
  while(!Serial){
    delay(1);
  }
  Serial.println("Initialising");


  if(!system_state_init()){
    state = HW_STATE_FAILED;
  } else if (!gpio_init()){
    state = HW_GPIO_INIT_FAILED;
  } else if(adc_init() != ADC_STATUS_OK) {
    state = HW_ADC_INIT_FAILED;
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
      fsm_handler_adc_failure();
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
  uint16_t raw_reading = 0;
  uint16_t calibration_reading = 0;
  uint16_t fo2 = 0;
  uint16_t mV = 0;
  uint16_t calibrated_fo2;

  if(system_state_get_cell_check_time(&last_check_time) != SYSTEM_OK){
    system_state_set_loop_state(STATE_FAILED_SAFE);
    return;
  }

  if(has_timer_elapsed(now, last_check_time, CELL_READ_FREQUENCY_MS)){
    if(adc_raw_reading(&raw_reading) != ADC_STATUS_OK){
      system_state_set_loop_state(STATE_ADC_FAILURE);
      return;
    }
    if(!gpio_read_calibration_pin(&calibration_reading)){
      system_state_set_loop_state(STATE_FAILED_SAFE);
      return;
    }
    if(system_state_set_cell_check_time(now) != SYSTEM_OK){
      system_state_set_loop_state(STATE_FAILED_SAFE);
      return;
    }
    fo2 = convert_raw_to_fo2(raw_reading);
    mV = adc_convert_raw_to_mV(raw_reading);
    calibrated_fo2 = calibrate_value_to_percent(fo2, calibration_reading);
    Serial.print(mV);
    Serial.print(" ");
    Serial.print(calibrated_fo2);
    Serial.print(" ");
    Serial.println(fo2);
  }
}

void fsm_handler_adc_failure(void){
  Serial.println("ADC FAILURE");
  delay(1000);
}

void fsm_handler_failed_safe(void){
  Serial.println("FAILED SAFE!");
  delay(1000);
}


bool has_timer_elapsed(const uint32_t current_time, const uint32_t last_time, const uint32_t frequency){
  return (current_time - last_time) >= frequency; // See Note 9
}



uint16_t calibrate_value_to_percent(uint16_t raw_reading, uint16_t potentiometer_reading){
  uint16_t calibration_factor = 0;
  uint16_t calibration_range = CAL_PERCENT_MAX - CAL_PERCENT_MIN;
  uint16_t pot_range = POT_MAX - POT_MIN;

  calibration_factor = CAL_PERCENT_MIN + ((uint32_t)(potentiometer_reading - POT_MIN) * calibration_range) / pot_range;

  return ((uint32_t)raw_reading * calibration_factor) / 100;
}


uint16_t convert_raw_to_fo2(const uint16_t raw_reading){
  const uint32_t numerator = (uint32_t)(raw_reading) * ADC_REFERENCE_FO2_X10;

  return (uint16_t)((numerator + (ADC_REFERENCE_RAW / 2U)) / ADC_REFERENCE_RAW);
}
