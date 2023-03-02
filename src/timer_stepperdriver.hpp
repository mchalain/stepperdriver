#ifndef __TIMER_STEPPERDRIVER_HPP__
#define __TIMER_STEPPERDRIVER_HPP__

class Timer
{
  public:
		virtual void start(int us) = 0;
		virtual void restart(int us) = 0;
		virtual void stop() = 0;
		virtual bool check() = 0;
		static Timer *makeTimer();
};

#endif
