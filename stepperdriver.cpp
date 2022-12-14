#include <Arduino.h>

#include "stepperdriver.hpp"

#define NEGATIVSENS 0x01
#define HOMING 0x02
#define STEPHIGH    0x10

Stepper::Stepper(int en, int step, int dir, unsigned int max, int end, int enState)
  : enPin(en), stepPin(step), dirPin(dir), endPin(end), _enable(enState), _position(0), _speed(500),
    _nbsteps(0), _max(-1),_state(0)
{
  pinMode(this->enPin,OUTPUT); 
  pinMode(this->stepPin,OUTPUT); 
  pinMode(this->dirPin,OUTPUT);
  if (this->endPin != -1)
    pinMode(this->endPin,INPUT);
  digitalWrite(this->enPin, !this->_enable);
  
}
int Stepper::turn(int nbsteps, int speed)
{
  if (enabled())
    return -1;
  int dir = LOW;
  if (nbsteps<0)
  {
    this->_state |= NEGATIVSENS;
    dir = HIGH;
  }
  else
  {
    this->_state &= ~NEGATIVSENS;
  }
  this->_nbsteps = (nbsteps<0)?-nbsteps:nbsteps;
  this->_speed = speed / 2;
  digitalWrite(this->dirPin, dir);
  return 0;
}
void Stepper::home()
{
  turn(-1 * this->_position);
  if (this->endPin != -1)
    this->_state |= HOMING;
}
void Stepper::start()
{
  if ((this->_nbsteps == 0) ||
    (this->_position == 0 && (this->_state & NEGATIVSENS)) ||
    (this->_position == this->_max && !(this->_state & NEGATIVSENS)))
  {
    return;
  }
  digitalWrite(this->enPin, this->_enable);
  this->_ptime = this->_time() + this->_speed;
}
int Stepper::_time()
{
  return micros();
}
int Stepper::_checktimer()
{
  int ret = (this->_ptime - this->_time()) <= 0;
  if (ret)
    this->_ptime += this->_speed;
  return ret;
}
void Stepper::stop()
{
  digitalWrite(this->enPin, !this->_enable);
  this->_nbsteps = 0;
}
int Stepper::enabled()
{
  return (digitalRead(this->enPin) == this->_enable);
}
unsigned int Stepper::position()
{
  return this->_position;
}
void Stepper::_handler()
{
  if (this->_state & STEPHIGH)
  {
    digitalWrite(this->stepPin,HIGH);
    this->_state &= ~STEPHIGH;
  }
  else
  {
    digitalWrite(this->stepPin,LOW);
    this->_state |= STEPHIGH;
    this->_position += (this->_state & NEGATIVSENS)? -1 : 1;
    this->_nbsteps--;
  }
}
int Stepper::step(int speed)
{
  if (!enabled())
    return  -1;
  if (this->endPin != -1 && (this->_state & NEGATIVSENS))
  {
    if (this->_state & HOMING)
    {
      this->_position = 1; // keep the position positiv while end sensor is LOW
      this->_nbsteps = 1;
    }
    if (digitalRead(this->endPin) == LOW)
    {
      stop();
      this->_position = 0;
      return -1;
    }
  }
  if ((this->_nbsteps == 0) ||
    (this->_position == 0 && (this->_state & NEGATIVSENS)) ||
    (this->_position == this->_max && !(this->_state & NEGATIVSENS)))
  {
    stop();
    return 0;
  }
  if (_checktimer())
  {
    _handler();
  }
  return this->_nbsteps;
}
int Stepper::step()
{
  return step(this->_speed);
}
