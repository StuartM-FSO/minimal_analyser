#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>
#include <stdint.h>

typedef enum{
  STATE_READ_CELL,
  STATE_HW_FAILURE,
  STATE_UNINITIALISED,
  STATE_START_UP,
  STATE_FAILED_SAFE
} loop_state_t;

typedef enum{
  HW_ZERO_COUNT = 0, // DO NOT ADD STATES BEFORE THIS
  HW_UNINITIALISED,
  HW_OK,
  HW_STATE_FAILED,
  HW_GPIO_INIT_FAILED,
  HW_ADC_INIT_FAILED,
  HW_DISPLAY_FAILED,
  HW_END_COUNT // DO NOT ADD STATES BEYOND THIS
} hw_state_t;

typedef enum{
  SYSTEM_OK,
  SYSTEM_INVALID_PARAMETER,
  SYSTEM_UNINITIALISED
} system_function_t;

bool system_state_init(void);

system_function_t system_state_set_loop_state(const loop_state_t state);
system_function_t system_state_get_loop_state(loop_state_t *state);

system_function_t system_state_get_loop_check_time(uint32_t *check_time);
system_function_t system_state_set_loop_check_time(const uint32_t check_time);

//system_function_t system_state_get_hw_state(hw_state_t *state);
//system_function_t system_state_set_hw_state(const hw_state_t state);

#endif