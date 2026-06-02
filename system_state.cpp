#include "system_state.h"

typedef struct{
  bool initialised = false;
  loop_state_t loop_state = STATE_UNINITIALISED;
  hw_state_t current_hw_state = HW_UNINITIALISED;
  uint32_t cell_check_time = 0U;
} system_state_t;



static system_state_t current_state;

bool is_valid_loop_state(loop_state_t state);

// Public

bool system_state_init(void){
  if(current_state.initialised){
    return true;
  }
  current_state.loop_state = STATE_READ_CELL;
  current_state.initialised = true;
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

system_function_t system_state_get_cell_check_time(uint32_t *check_time){
  if(!current_state.initialised){
    return SYSTEM_UNINITIALISED;
  }
  if(check_time == NULL){
    return SYSTEM_INVALID_PARAMETER;
  }
  *check_time = current_state.cell_check_time;
  return SYSTEM_OK;
}

system_function_t system_state_set_cell_check_time(const uint32_t check_time){
  current_state.cell_check_time = check_time;
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