#ifndef __HAL_STEPPERDRIVER_H__
#define __HAL_STEPPERDIRVER_H__

#ifdef ARDUINO
#include <Arduino.h>
#include "../hal/arduino/gpio.hpp"
#include "../hal/arduino/timer.hpp"

#ifdef DEBUG
#define debug(format,... ) Serial.printf(format"\r\n", ##__VA_ARGS__)
#else
#define debug(...)
#endif
#endif

#endif
