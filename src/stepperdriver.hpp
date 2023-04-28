#ifndef __STEPPERDRIVER_HPP__
#define __STEPPERDRIVER_HPP__

#include "gpio_stepperdriver.hpp"
#include "timer_stepperdriver.hpp"

#define LINEARMOVEMENT 0x01
#define CIRCULARMOVEMENT 0x02

class Stepper
{
  GeneralOutput *enPin;
  GeneralOutput *stepPin;
  GeneralOutput *dirPin;
  GeneralInput *endPin;
  int _max;
  int _position;
  int _nbsteps;
  unsigned short int _speed;
  unsigned short int _stepsmm;
  char _state;
  bool _enable;
  Timer *timer;

  class Move
  {
      unsigned short int _targetspeed;
    public:
      virtual void settargetspeed(unsigned short int speed)
      { _targetspeed = speed;}
      virtual unsigned short int speed(unsigned short int speed, int nbsteps) = 0;
  };
  class Linear: public Move
  {
    friend Stepper;
    protected:
      unsigned short int _accel;
      unsigned short int _minspeed;
      unsigned short int _maxspeed;
      unsigned short int _speedtarget;
    public:
	  Linear(unsigned short int maxspeed, unsigned short int minspeed, unsigned short int accel);
      void settargetspeed(unsigned short int speed);
      unsigned short int speed(unsigned short int speed, int nbsteps);
  };
  class Circular: public Move
  {
    friend Stepper;
    protected:
      unsigned short int _accel;
      unsigned short int _minspeed;
      unsigned short int _maxspeed;
      unsigned short int _speedtarget;
      int _nbsteps;
    public:
	  Circular(unsigned short int maxspeed, unsigned short int minspeed, unsigned short int accel);
      void settargetspeed(unsigned short int speed);
      unsigned short int speed(unsigned short int speed, int nbsteps);
  };
  Linear *_linear;
  Circular *_circular;
  Move *_move;
  int _time();
  int _checktimer();
  void _settimer(int latence);
  void _handler();
public:
  enum Setting
  {
    MaxPosition,
    Accel,
    MaxSpeed,
    StepsPerMilliMeter,
    MilliMeterMode,
    Movement,
  };
  Stepper(int en, int step, int dir, unsigned int max, int end = -1, bool enState = false);
  void setup(Setting setting, int value);
  int setup(Setting setting);
  int turn(int distance, int speed = 4000);
  int move(int distance, int hypotenuse, int speed = 4000);
  void changedirection(int dir);
  void home(int speed = 2000);
  int step(int speed);
  int step();
  void start();
  void stop();
  int enabled();
  unsigned int position();
  unsigned int max();
};
#endif
