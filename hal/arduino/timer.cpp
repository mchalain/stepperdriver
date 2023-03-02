#include <Arduino.h>

#include "timer.hpp"

HalTimer::HalTimer()
	: Timer(), latence(0), iter(1), starttime(0)
{
}

HalTimer::~HalTimer()
{
	this->stop();
}

int HalTimer::time()
{
	return micros();
}

void HalTimer::start(int us)
{
	this->latence = us;
	this->starttime = this->time();
}

void HalTimer::restart(int us)
{
	this->starttime += this->latence;
	this->latence = us;
}

void HalTimer::stop()
{
	this->starttime = 0;
	this->iter = 1;
}

bool HalTimer::check()
{
	if (this->starttime == 0)
		return false;
	bool ret = ((int)(this->starttime + this->latence) - this->time()) <= 0;
	return ret;
}

Timer *Timer::makeTimer()
{
	return new HalTimer();
}
