#include "Arduino.h"
#include "gpio_hal.h"
#include <stdint.h>

static const uint8_t CALIBRATION_PIN = A3;

static bool initialised = false;


bool gpio_init(void){
  if(initialised){
    return true;
  }
  initialised = true;
  return true;
}

bool gpio_read_calibration_pin(uint16_t *read_val){
  if(!initialised){
    return false;
  }
  if(read_val == NULL){
    return false;
  }
  *read_val = analogRead(CALIBRATION_PIN);
  Serial.println(*read_val);
  return true;
}