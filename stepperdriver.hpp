#ifndef __STEPPERDRIVER_HPP__
#define __STEPPERDRIVER_HPP__

class Stepper
{
  int enPin;
  int stepPin;
  int dirPin;
  int endPin;
  int _max;
  int _position;
  int _nbsteps;
  unsigned short int _accel;
  unsigned short int _minspeed;
  unsigned short int _maxspeed;
  unsigned short int _speed;
  unsigned short int _speedtarget;
  unsigned short int _stepsmm;
  char _state;
  char _enable;
  int _ptime;

  int _time();
  int _checktimer();
  void _handler();
public:
  enum Setting
  {
    MaxPosition,
    Accel,
    MaxSpeed,
    StepsPerMilliMeter,
    MilliMeterMode,
  };
  Stepper(int en, int step, int dir, unsigned int max, int end = -1, int enState = LOW);
  void setup(Setting setting, int value);
  int turn(int nbsteps, int speed = 4000);
  void home(int speed = 2000);
  int step(int speed);
  int step();
  void start();
  void stop();
  int enabled();
  unsigned int position();
};
#endif
