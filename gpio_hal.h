#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <Arduino.h>
#include <stdint.h>

bool gpio_init(void);
bool gpio_read_calibration_pin(uint16_t *read_val);

#endif