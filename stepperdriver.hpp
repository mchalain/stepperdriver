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
  unsigned short int _speed;
  char _state;
  char _enable;
  int _ptime;

  int _time();
  int _checktimer();
  void _handler();
public:
  Stepper(int en, int step, int dir, unsigned int max, int end = -1, int enState = LOW);
  int turn(int nbsteps, int speed = 500);
  void home();
  int step(int speed);
  int step();
  void start();
  void stop();
  int enabled();
  unsigned int position();
};
#endif
