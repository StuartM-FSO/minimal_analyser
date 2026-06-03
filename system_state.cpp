#include "system_state.h"

typedef struct{
  bool initialised = false;
  loop_state_t loop_state = STATE_UNINITIALISED;
  uint32_t loop_check_time = 0U;
  bool run_once = false;
  bool pulse_on = false;
  uint32_t pulse_time = 0U;
} system_state_t;



static system_state_t current_state;

bool is_valid_loop_state(loop_state_t state);
//bool is_valid_hw_state(const hw_state_t state);

// Public

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
  switch (state) {
    case STATE_READ_CELL:
    case STATE_HW_FAILURE:
    case STATE_UNINITIALISED:
    case STATE_START_UP:
    case STATE_FAILED_SAFE:
      return true;
    default:
      return false;
  }
}