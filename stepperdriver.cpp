#include <Arduino.h>

#include "stepperdriver.hpp"

#define TOGGLESENS 0xF1
#define POSITIVSENS 0x00
#define NEGATIVSENS 0x01
#define HOMING 0x02
#define MILLIMODE 0x04
#define STEPHIGH    0x10

#define MAXSTEPS 1000000 /// number of steps per seconds

Stepper::Stepper(int en, int step, int dir, unsigned int max, int end, int enState)
  : enPin(en), stepPin(step), dirPin(dir), endPin(end), _enable(enState), _max(max), _state(0),
    _position(0), _speed(0), _nbsteps(0),
    _stepsmm(1)
{
	pinMode(this->enPin,OUTPUT);
	pinMode(this->stepPin,OUTPUT);
	pinMode(this->dirPin,OUTPUT);
	if (this->endPin != -1)
		pinMode(this->endPin,INPUT);
	digitalWrite(this->enPin, !this->_enable);
	this->_linear = new Linear(2000, 0, 10);
	this->_move = this->_linear;
}
void Stepper::setup(Stepper::Setting setting, int value)
{
	switch (setting)
	{
		case Stepper::Movement:
			this->_move = this->_linear;
		break;
		case Stepper::Accel:
			this->_linear->_accel = value;
		break;
		case Stepper::MaxSpeed:
			this->_linear->_maxspeed = value;
		break;
		case Stepper::MaxPosition:
			this->_max = value;
		break;
		case Stepper::StepsPerMilliMeter:
			this->_stepsmm = value;
		break;
		case Stepper::MilliMeterMode:
			if (value)
				this->_state |= MILLIMODE;
			else
				this->_state &= ~MILLIMODE;
		break;
	}
}
void Stepper::changedirection(int dir)
{
	if (dir == TOGGLESENS)
	{
		dir = (this->_state & NEGATIVSENS)?0:NEGATIVSENS;
	}
	if (dir == NEGATIVSENS)
	{
		this->_state |= NEGATIVSENS;
		digitalWrite(this->dirPin, HIGH);
	}
	else
	{
		this->_state &= ~NEGATIVSENS;
		digitalWrite(this->dirPin, LOW);
	}
}
int Stepper::turn(int nbsteps, int speed)
{
	if (enabled())
	return -1;
	int dir = 0;
	if (nbsteps<0)
	{
		dir = NEGATIVSENS;
		this->_nbsteps = -nbsteps;
	}
	else
	{
		this->_nbsteps = nbsteps;
	}
	if (this->_state & MILLIMODE)
		speed /= this->_stepsmm;
	changedirection(dir);
	this->_move->settargetspeed(speed);
	this->_speed = this->_move->speed(0, this->_nbsteps);
	return 0;
}
void Stepper::home(int speed)
{
	if (this->endPin != -1)
		this->_position = 1;
	turn(-1 * this->_position, speed);
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
	this->_ptime = this->_time() + ((MAXSTEPS / 2) / this->_speed);
}
int Stepper::_time()
{
	return micros();
}
Stepper::Linear::Linear(unsigned short int maxspeed, unsigned short int minspeed, unsigned short int accel)
	: _maxspeed(maxspeed), _minspeed(minspeed), _accel(accel)
{
	if (this->_minspeed < this->_accel)
		this->_minspeed = this->_accel;
}
void Stepper::Linear::settargetspeed(unsigned short int speed)
{
	this->_speedtarget = (speed < this->_maxspeed)?speed:this->_maxspeed;
}
unsigned short int Stepper::Linear::speed(unsigned short int speed, int nbsteps)
{
	if (speed == 0)
		speed = this->_minspeed;
	if (nbsteps < (speed / this->_accel))
		speed -= this->_accel;
	else if (speed < this->_speedtarget)
		speed += this->_accel;
	return speed;
}
int Stepper::_checktimer()
{
	int ret = (this->_ptime - this->_time()) <= 0;
	return ret;
}
void Stepper::_settimer(int latence)
{
	this->_ptime += latence;
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
unsigned int Stepper::max()
{
	return this->_max;
}
void Stepper::_handler()
{
	_settimer((MAXSTEPS / 2) / this->_speed);
	unsigned short int oldspeed = this->_speed;
	this->_speed = this->_move->speed(this->_speed, this->_nbsteps);
	if (this->_speed == 0xFFFF)
	{
		this->_speed = oldspeed;
		if (this->_state & STEPHIGH)
		{
			digitalWrite(this->enPin, !this->_enable);
			changedirection(TOGGLESENS);
			digitalWrite(this->enPin, this->_enable);
		}
	}
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
		return  0;
	if (this->endPin != -1 && (this->_state & NEGATIVSENS))
	{
		if (this->_state & HOMING)
		{
			this->_position = 1; // keep the position positiv while end sensor is LOW
			this->_nbsteps = this->_max;
		}
		if (digitalRead(this->endPin) == LOW)
		{
			stop();
			this->_position = 0;
			this->_state &= ~HOMING;
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
