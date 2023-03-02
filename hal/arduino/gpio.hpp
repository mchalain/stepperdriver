#ifndef __HAL_GPIO_STEPPERDIRVER_H__
#define __HAL_GPIO_STEPPERDIRVER_H__

#include "gpio_stepperdriver.hpp"

class HalDigiInput : public GeneralInput
{
    int number;
  public:
    HalDigiInput(int chip, int number);
    virtual ~HalDigiInput();
    bool value();
};

class HalDigiOutput : public GeneralOutput
{
    int number;
  public:
    HalDigiOutput(int chip, int number);
    virtual ~HalDigiOutput();
    bool value();
    void value(bool change);
};

#include "../hal/arduino/gpio.cpp"

#endif
