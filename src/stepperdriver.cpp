#include "stepperdriver.hpp"
#include "hal_stepperdriver.hpp"

#define TOGGLESENS 0xF1
#define POSITIVSENS 0x00
#define NEGATIVSENS 0x01
#define HOMING 0x02
#define MILLIMODE 0x04
#define FORCE_ENABLE 0x08
#define STEPHIGH    0x10

#define MAXSTEPS 1000000 /// number of steps per seconds

#define DEBUGSTR do { debug("%s %d\r\n", __FUNCTION__, __LINE__);}while(0)

Stepper::Stepper(int en, int step, int dir, unsigned int max, int end, bool enState)
  : enPin(nullptr), stepPin(nullptr), dirPin(nullptr), endPin(nullptr), _enable(enState), _max(max), _state(0),
    _position(0), _speed(1), _nbsteps(0),
    _stepsmm(1)
{
	this->enPin = GeneralOutput::makeGeneralOutput(0, en);
	this->stepPin = GeneralOutput::makeGeneralOutput(0, step);
	this->dirPin = GeneralOutput::makeGeneralOutput(0, dir);
	if (end > -1)
		this->endPin = GeneralInput::makeGeneralInput(0, end);
	else if (end < -1)
		this->endPin = GeneralInput::makeGeneralInput(0, -end, true);
	this->enPin->value(!this->_enable);
	this->_linear = new Linear(2000, 0, 10);
	this->_circular = new Circular(2000, 0, 10);
	this->_move = this->_linear;
	this->timer = Timer::makeTimer();
}
int Stepper::setup(Stepper::Setting setting)
{
	int ret = -1;
	switch (setting)
	{
		case Stepper::Movement:
			if (this->_move == this->_circular)
				ret = CIRCULARMOVEMENT;
			else
				ret = LINEARMOVEMENT;
		break;
		case Stepper::Accel:
			ret = this->_linear->_accel;
		break;
		case Stepper::MaxSpeed:
			ret = this->_linear->_maxspeed;
		break;
		case Stepper::MaxPosition:
			ret = this->_max;
		break;
		case Stepper::StepsPerMilliMeter:
			ret = this->_stepsmm;
		break;
		case Stepper::MilliMeterMode:
			ret = !((this->_state & MILLIMODE) == 0);
		break;
		case Stepper::ForceEnable:
			ret = !((this->_state & FORCE_ENABLE) == 0);
		break;
	}
	return ret;
}
void Stepper::setup(Stepper::Setting setting, int value)
{
	switch (setting)
	{
		case Stepper::Movement:
			if (value == CIRCULARMOVEMENT)
				this->_move = this->_circular;
			else if (value == LINEARMOVEMENT)
				this->_move = this->_linear;
		break;
		case Stepper::Accel:
			if (this->_state & MILLIMODE)
				value *= this->_stepsmm;
			if (value > 0)
				this->_linear->_accel = value;
		break;
		case Stepper::MaxSpeed:
			if (this->_state & MILLIMODE)
				value *= this->_stepsmm;
			if (value > 0)
				this->_linear->_maxspeed = value;
		break;
		case Stepper::MaxPosition:
			if (this->_state & MILLIMODE)
				value *= this->_stepsmm;
			if (value > 1)
				this->_max = value;
		break;
		case Stepper::StepsPerMilliMeter:
			if (value > 0)
				this->_stepsmm = value;
		break;
		case Stepper::MilliMeterMode:
			if (value > 0)
				this->_state |= MILLIMODE;
			else if (value == 0)
				this->_state &= ~MILLIMODE;
		break;
		case Stepper::ForceEnable:
			if (value > 0)
				this->_state |= FORCE_ENABLE;
			else if (value == 0)
				this->_state &= ~FORCE_ENABLE;
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
	}
	else
	{
		this->_state &= ~NEGATIVSENS;
	}
	this->dirPin->value(this->_state & NEGATIVSENS);
}
int Stepper::move(int distance, int hypotenuse, int speed)
{
	if (enabled())
		return -1;
	int dir = 0;
	if (distance < 0)
	{
		dir = NEGATIVSENS;
		distance = -distance;
	}
	if (hypotenuse != 0)
	{
		speed *= distance;
		speed /= hypotenuse;
	}
	changedirection(dir);
	this->_nbsteps = distance;
	if (this->_state & MILLIMODE)
	{
		this->_nbsteps *= this->_stepsmm;
		speed *= this->_stepsmm;
	}
	this->_move->settargetspeed(speed);
	this->_speed = this->_move->speed(0, this->_nbsteps);
	return 0;
}
int Stepper::turn(int distance, int speed)
{
	return this->move(distance, 0, speed);
}
void Stepper::home(int speed)
{
	if (this->endPin != nullptr)
	{
		this->_position = 1;
		this->_state |= HOMING;
	}
	turn(-1 * this->_position, speed);
}
void Stepper::start()
{
	if ((this->_nbsteps == 0) ||
		(this->_position == 0 && (this->_state & NEGATIVSENS)) ||
		(this->_position == this->_max && !(this->_state & NEGATIVSENS)))
	{
		return;
	}
	this->enPin->value(this->_enable);
	this->timer->start((MAXSTEPS / 2) / this->_speed);
}
void Stepper::stop(int disable)
{
	this->timer->stop();
	this->stepPin->value(false);
	if (!(this->_state & FORCE_ENABLE) || disable)
	{
		this->enPin->value(!this->_enable);
	}
	this->_nbsteps = 0;
}
int Stepper::enabled()
{
	return (this->enPin->value() == this->_enable);
}
unsigned int Stepper::position()
{
	unsigned int position = this->_position;
	if (this->_state & MILLIMODE)
	{
		position /= this->_stepsmm;
	}
	return position;
}
unsigned int Stepper::max()
{
	return this->_max;
}
void Stepper::_handler()
{
	unsigned short int oldspeed = this->_speed;
	this->_speed = this->_move->speed(this->_speed, this->_nbsteps);
	if (this->_speed == 0xFFFF)
	{
		this->_speed = oldspeed;
		if (this->_state & STEPHIGH)
		{
			this->enPin->value(!this->_enable);
			changedirection(TOGGLESENS);
			this->enPin->value(this->_enable);
		}
	}
	if (this->_state & STEPHIGH)
	{
		this->stepPin->value(true);
		this->_state &= ~STEPHIGH;
	}
	else
	{
		this->stepPin->value(false);
		this->_state |= STEPHIGH;
		this->_position += (this->_state & NEGATIVSENS)? -1 : 1;
		this->_nbsteps--;
	}
}
int Stepper::step(int speed)
{
	if (!enabled())
		return  0;
	if (this->endPin != nullptr && (this->_state & NEGATIVSENS))
	{
		if (this->_state & HOMING)
		{
			this->_position = this->_max; // keep the position positiv while end sensor is LOW
			this->_nbsteps = this->_max;
		}
		if (this->endPin->value())
		{
			stop();
			this->_position = 0;
			if (this->_state & HOMING)
			{
				this->_state &= ~HOMING;
				return 0;
			}
			return -1;
		}
	}
	if (this->timer->check())
	{
		this->timer->restart((MAXSTEPS / 2) / this->_speed);
		_handler();
	}
	if ((this->_nbsteps == 0) ||
		(this->_position == 0 && (this->_state & NEGATIVSENS)) ||
		(this->_position == this->_max && !(this->_state & NEGATIVSENS)))
	{
		stop();
		return 0;
	}
	return this->_nbsteps;
}
int Stepper::step()
{
	return step(this->_speed);
}
/***********************************/
Stepper::Circular::Circular(unsigned short int maxspeed, unsigned short int minspeed, unsigned short int accel)
	: _maxspeed(maxspeed), _minspeed(minspeed), _accel(accel)
{
	if (this->_minspeed < this->_accel)
		this->_minspeed = this->_accel;
}
void Stepper::Circular::settargetspeed(unsigned short int speed)
{
	this->_speedtarget = (speed < this->_maxspeed)?speed:this->_maxspeed;
}
unsigned short int Stepper::Circular::speed(unsigned short int speed, int nbsteps)
{
	if (speed == 0)
	{
		speed = this->_speedtarget;
		this->_nbsteps = nbsteps;
	}
	speed = (int)((float)this->_speedtarget * sin(TWO_PI * (float)nbsteps/(float)this->_nbsteps));
	debug("%f   %f", (float)nbsteps/(float)this->_nbsteps, sin(TWO_PI * (float)(nbsteps - 1)/(float)this->_nbsteps));
	speed = (speed < 0)? -speed:speed;
	if (speed == this->_speedtarget)
		debug("PI/2\n");
	speed++;
	if ((this->_nbsteps / 2) == nbsteps)
	{
		speed = -1;
	}
	return speed;
}
/***********************************/
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
