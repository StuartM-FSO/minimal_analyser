#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>
#include <stdint.h>

typedef enum{
  STATE_READ_CELL,
  STATE_ADC_FAILURE,
  STATE_UNINITIALISED,
  STATE_START_UP,
  STATE_FAILED_SAFE
} loop_state_t;

typedef enum{
  HW_UNINITIALISED,
  HW_OK,
  HW_STATE_FAILED,
  HW_GPIO_INIT_FAILED
} hw_state_t;

typedef enum{
  SYSTEM_OK,
  SYSTEM_INVALID_PARAMETER,
  SYSTEM_UNINITIALISED
} system_function_t;

bool system_state_init(void);

system_function_t system_state_set_loop_state(const loop_state_t state);
system_function_t system_state_get_loop_state(loop_state_t *state);

system_function_t system_state_get_cell_check_time(uint32_t *check_time);
system_function_t system_state_set_cell_check_time(const uint32_t check_time);

#endif