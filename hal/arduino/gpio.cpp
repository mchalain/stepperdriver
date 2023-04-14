#include <Arduino.h>

#include "gpio.hpp"

HalDigiInput::HalDigiInput(int chip, int number, bool inverted)
	: GeneralInput(), number(number), inverted(inverted)
{
	pinMode(number, INPUT);
}

HalDigiInput::~HalDigiInput()
{
}

bool HalDigiInput::value()
{
	bool value = digitalRead(this->number);
	if (inverted)
		return !value;
	return value;
}

GeneralInput *GeneralInput::makeGeneralInput(int chip, int number, bool inverted)
{
	return new HalDigiInput(chip, number, inverted);
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
