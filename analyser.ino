#include "adc_hal.h"
#include "display_hal.h"
#include "system_state.h"
#include "gpio_hal.h"
#include <Wire.h>

static const uint32_t CELL_READ_FREQUENCY_MS = 250U;

const uint16_t POT_MIN = 0U;
const uint16_t POT_MAX = 700U;

const uint16_t CAL_PERCENT_MIN = 75U;
const uint16_t CAL_PERCENT_MAX = 125U;

const uint16_t ADC_REFERENCE_RAW = 1600U; // for high output cells use 3200
const uint16_t ADC_REFERENCE_FO2_X10 = 210U;

const uint16_t MAXIMUM_PPO2_X1000 = 16000U;

const uint16_t DEPTH_CORRECTION_FACTOR = 10U;

const uint16_t HYPOXIC_THRESHOLD_X10 = 170U;

const uint32_t ONE_SECOND_MS = 1000U;



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
  } else if(display_init() != DISPLAY_STATUS_OK) {
    state = HW_DISPLAY_FAILED;
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
    case HW_DISPLAY_FAILED:
      system_state_set_loop_state(STATE_FAILED_SAFE);
      break;
    case HW_ADC_INIT_FAILED:
    case HW_GPIO_INIT_FAILED:
    case HW_STATE_FAILED:
      system_state_set_loop_state(STATE_HW_FAILURE);
      break;
    default:
      system_state_set_loop_state(STATE_FAILED_SAFE);
      break;
  }
  
}

void loop() {
  loop_state_t state;
  if(system_state_get_loop_state(&state) != SYSTEM_OK){
    system_state_set_loop_state(STATE_FAILED_SAFE);
    return;
  }
  uint32_t now = millis();

  switch (state) {
    case STATE_START_UP:
      fsm_handler_start_up();
      break;
    case STATE_READ_CELL:
      fsm_handler_read_cell(now);
      break;
    case STATE_HW_FAILURE:
      fsm_handler_hw_failure();
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
  uint32_t last_time = 0;
  uint32_t now = millis();

  if(system_state_get_loop_check_time(&last_time) != SYSTEM_OK){
    system_state_set_loop_state(STATE_HW_FAILURE);
    return;
  }
  if(has_timer_elapsed(now, last_time, ONE_SECOND_MS)){
    system_state_set_loop_state(STATE_READ_CELL);
    return;
  }
  if(!system_state_get_run_once_flag()){
    Serial.println("Start up");
    display_clear();
    display_font_size(2);
    display_set_colour(DISPLAY_WHITE, DISPLAY_BLACK);
    display_set_cursor(0, 0);
    display_print("START");
    display_update();
    system_state_set_run_once_flag(true);
  }
}

void fsm_handler_read_cell(const uint32_t now){
  uint32_t last_check_time = 0U;
  uint32_t pulse_check_time = 0U;
  uint16_t raw_reading = 0;
  uint16_t calibration_reading = 0;
  uint16_t fo2 = 0;
  uint16_t mV = 0;
  uint16_t calibrated_fo2;
  uint16_t mod_msw = 0;
  char buffer_fo2[6] = {""};
  char buffer_mod[4] = {""};
  char buffer_mv[4] = {""};

  if(system_state_get_loop_check_time(&last_check_time) != SYSTEM_OK){
    system_state_set_loop_state(STATE_FAILED_SAFE);
    return;
  }

  if(has_timer_elapsed(now, last_check_time, CELL_READ_FREQUENCY_MS)){
    if(adc_raw_reading(&raw_reading) != ADC_STATUS_OK){
      system_state_set_loop_state(STATE_HW_FAILURE);
      return;
    }
    if(!gpio_read_calibration_pin(&calibration_reading)){
      system_state_set_loop_state(STATE_FAILED_SAFE);
      return;
    }
    if(system_state_set_loop_check_time(now) != SYSTEM_OK){
      system_state_set_loop_state(STATE_FAILED_SAFE);
      return;
    }
    fo2 = convert_raw_to_fo2(raw_reading);
    mV = adc_convert_raw_to_mV(raw_reading);
    calibrated_fo2 = calibrate_value_to_percent(fo2, calibration_reading);
    mod_msw = calculate_mod(calibrated_fo2);
    format_fo2_for_display(calibrated_fo2, buffer_fo2);
    format_integer_for_display(mod_msw, buffer_mod);
    format_integer_for_display(mV, buffer_mv);

    display_clear();
    display_set_cursor(0, 0);
    display_print("fO2 ");
    if(calibrated_fo2 < HYPOXIC_THRESHOLD_X10){
      display_set_colour(DISPLAY_BLACK, DISPLAY_WHITE);
    }
    display_print(buffer_fo2);
    display_println("%");
    display_set_colour(DISPLAY_WHITE, DISPLAY_BLACK);
    display_print("MOD");
    if(mod_msw < 10){
      display_print("   ");
    } else if (mod_msw > 99){
      display_print(" ");
    } else {
      display_print("  ");
    }
    display_print(buffer_mod);
    display_println("msw");
    //display_font_size(1);
    display_print("Cell");
    if(mV < 10){
      display_print("   ");
    } else if (mV > 99){
      display_print(" ");
    } else {
      display_print("  ");
    }
    display_print(buffer_mv);
    display_println("mV");
    if(system_state_get_pulse_on_flag()){
      display_print("+");
    }
    system_state_get_pulse_check_time(&pulse_check_time);
    if(has_timer_elapsed(now, pulse_check_time, ONE_SECOND_MS)){
      system_state_invert_pulse_on_flag();
      system_state_set_pulse_check_time(now);
    }

    display_update();
  }
}

void fsm_handler_hw_failure(void){
  display_clear();
  display_set_cursor(0, 0);
  display_print("HW FAILED");
  display_update();
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

  if(potentiometer_reading < POT_MIN){
    potentiometer_reading = POT_MIN;
  }
  if(potentiometer_reading > POT_MAX){
    potentiometer_reading = POT_MAX;
  }
  calibration_factor = CAL_PERCENT_MIN + ((uint32_t)(potentiometer_reading - POT_MIN) * calibration_range) / pot_range;

  return ((uint32_t)raw_reading * calibration_factor) / 100;
}


uint16_t convert_raw_to_fo2(const uint16_t raw_reading){
  const uint32_t numerator = (uint32_t)(raw_reading) * ADC_REFERENCE_FO2_X10;

  return (uint16_t)((numerator + (ADC_REFERENCE_RAW / 2U)) / ADC_REFERENCE_RAW);
}

uint16_t calculate_mod(const uint16_t fo2){
  uint16_t rounded_down = fo2 / DEPTH_CORRECTION_FACTOR * DEPTH_CORRECTION_FACTOR;
  if(rounded_down == 0U){
    return 0U;
  }
  return MAXIMUM_PPO2_X1000 / rounded_down - DEPTH_CORRECTION_FACTOR;
}


void format_fo2_for_display(uint16_t fo2, char buffer[6]){
  uint16_t integer_part = 0;
  uint16_t temp = 0;
  uint8_t digit = 0;
  //uint8_t pos = 0;

  if (fo2 > 9999U){
    fo2 = 9999U;
  }
  integer_part = fo2 / 10U;

  /* Hundreds digit */
  temp = integer_part;
  digit = (uint8_t)(temp / 100U);
  buffer[0] = (char)('0' + digit);

  temp %= 100U;

  /* Tens digit */
  digit = (uint8_t)(temp / 10U);
  buffer[1] = (char)('0' + digit);

  /* Units digit */
  digit = (uint8_t)(temp % 10U);
  buffer[2] = (char)('0' + digit);

  buffer[3] = '.';

  /* Tenths digit */
  digit = (uint8_t)(fo2 % 10U);
  buffer[4] = (char)('0' + digit);

  buffer[5] = '\0';

  if (buffer[0] == '0')
  {
      buffer[0] = ' ';
  }

  if ((buffer[0] == ' ') && (buffer[1] == '0'))
  {
      buffer[1] = ' ';
  }

  //pos = 0U;
  //(void)pos; /* Silence unused-variable warning if MISRA checker requires it */
}

void format_integer_for_display(uint16_t value, char buffer[4]){
  uint16_t temp;
  uint8_t pos = 0U;

  if(value > 999U){
    value = 999U;
  }
  if(value >= 100U){
    buffer[pos] = (char)('0' + (uint8_t)(value / 100U));
    pos++;
    value %= 100U;
  }
  if((value >= 10U) || (pos > 0U)){
    buffer[pos] = (char)('0' + (uint8_t)(value / 10U));
    pos++;
    value %= 10U;
  }
  temp = value;
  buffer[pos] = (char)('0' + (uint8_t)temp);
  pos++;
  buffer[pos] = '\0';
}
