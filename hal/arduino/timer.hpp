#ifndef __HAL_TIMER_STEPPERDIRVER_H__
#define __HAL_TIMER_STEPPERDIRVER_H__

#include "timer_stepperdriver.hpp"

class HalTimer : public Timer
{
		int time();
		unsigned int starttime;
		unsigned int latence;
		unsigned int iter;
	public:
		HalTimer();
		virtual ~HalTimer();
		void start(int us);
		void restart(int us);
		void stop();
		bool check();
};

#include "../hal/arduino/timer.cpp"

#endif
