//#include "HardwareSerial.h"
#include "system_state.h"

typedef struct{
  bool initialised = false;
  loop_state_t loop_state = STATE_UNINITIALISED;
  uint32_t loop_check_time = 0U;
  bool run_once = false;
  bool pulse_on = false;
  uint32_t pulse_time = 0U;
} system_state_t;

typedef struct{
  bool fail_safe_entered = false;
  uint32_t last_check_time = 0;
  bool led_on = false;
} failsafe_t;



static system_state_t current_state;
static failsafe_t failed;

bool is_valid_loop_state(loop_state_t state);
//bool is_valid_hw_state(const hw_state_t state);

// Public

void failure_state_init(const uint32_t now){
  if(failed.fail_safe_entered){
    return;
  }
  failed.fail_safe_entered = true;
  failed.last_check_time = now;
  failed.led_on = true;
}

uint32_t failure_state_get_time(void){
  return failed.last_check_time;
}

void failure_state_set_time(const uint32_t now){
  failed.last_check_time = now;
}

bool failure_state_get_led_on(void){
  return failed.led_on;
}

void failure_state_set_led_on(const bool on_flag){
  failed.led_on = on_flag;
}

bool failure_state_is_failed(void){
  return failed.fail_safe_entered;
}

bool system_state_init(void){
  if(current_state.initialised){
    return true;
  }
  current_state.loop_state = STATE_READ_CELL;
  current_state.initialised = true;
  current_state.run_once = false;
  return true;
}

system_function_t system_state_set_loop_state(const loop_state_t state){
  if(!current_state.initialised){
    return SYSTEM_UNINITIALISED;
  }
  if(!is_valid_loop_state(state)){
    return SYSTEM_INVALID_PARAMETER;
  }
  current_state.loop_state = state;
  return SYSTEM_OK;
}

system_function_t system_state_get_loop_state(loop_state_t *state){
  if(state == NULL){
    return SYSTEM_INVALID_PARAMETER;
  }
  *state = current_state.loop_state;
  return SYSTEM_OK;
}

system_function_t system_state_get_loop_check_time(uint32_t *check_time){
  if(!current_state.initialised){
    return SYSTEM_UNINITIALISED;
  }
  if(check_time == NULL){
    return SYSTEM_INVALID_PARAMETER;
  }
  *check_time = current_state.loop_check_time;
  return SYSTEM_OK;
}

system_function_t system_state_set_loop_check_time(const uint32_t check_time){
  current_state.loop_check_time = check_time;
  return SYSTEM_OK;
}

bool system_state_get_run_once_flag(void){
  return current_state.run_once;
}

void system_state_set_run_once_flag(bool flag){
  current_state.run_once = flag;
}

bool system_state_get_pulse_on_flag(void){
  return current_state.pulse_on;
}

void system_state_invert_pulse_on_flag(void){
  current_state.pulse_on = !current_state.pulse_on;
}

system_function_t system_state_get_pulse_check_time(uint32_t *check_time){
  if(!current_state.initialised){
    return SYSTEM_UNINITIALISED;
  }
  if(check_time == NULL){
    return SYSTEM_INVALID_PARAMETER;
  }
  *check_time = current_state.pulse_time;
  return SYSTEM_OK;
}

system_function_t system_state_set_pulse_check_time(const uint32_t check_time){
  if(!current_state.initialised){
    return SYSTEM_UNINITIALISED;
  }
  current_state.pulse_time = check_time;
  return SYSTEM_OK;
}

// Private

bool is_valid_loop_state(loop_state_t state){
  return (state > STATE_COUNT_ZERO && state < STATE_COUNT_END);
}