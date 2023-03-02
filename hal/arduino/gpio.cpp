#include <Arduino.h>

#include "gpio.hpp"

HalDigiInput::HalDigiInput(int chip, int number)
	: GeneralInput(), number(number)
{
	pinMode(this->number, INPUT);
}

HalDigiInput::~HalDigiInput()
{
}

bool HalDigiInput::value()
{
	return digitalRead(this->number);
}

GeneralInput *GeneralInput::makeGeneralInput(int chip, int number)
{
	return new HalDigiInput(chip, number);
}

HalDigiOutput::HalDigiOutput(int chip, int number) : GeneralOutput(), number(number)
{
	pinMode(this->number, OUTPUT);
}

HalDigiOutput::~HalDigiOutput()
{
}

bool HalDigiOutput::value()
{       
	return digitalRead(this->number);
}       

void HalDigiOutput::value(bool change)
{
	digitalWrite(this->number, change?HIGH:LOW);
}

GeneralOutput *GeneralOutput::makeGeneralOutput(int chip, int number)
{
	return new HalDigiOutput(chip, number);
}
