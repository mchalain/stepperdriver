StepperDriver:
---

This library allow to use a driver for stepper motor.

# Software compatibility:

OS support:
 * Arduino all architectures

# Hardware compatibility:

 * Raspberry pico

# SDK

It is a C++ library

## Constructor:
```C++
Stepper::Stepper(int en, int step, int dir, unsigned int max, int end = -1, int enState = false);
```

 * _en_: GPIO number to enable the motor
 * _step_: GPIO number to drive a step on the motor
 * _dir_: GPIO number to drive the direction of the motor
 * _max_: the maximum numbers of steps for moving
 * _end_: GPIO number of the end sensor (endstop). It may be negative if the button is enable low.
 * _enState_: the level on the _en_ GPIO to enable the motor

example:
```C++
Stepper *stepper = new Stepper(10, 11, 12, 14000, 13);
```
## Prepare the deplacement
```C++
int Stepper::turn(int nbsteps, int speed = 4000);
```

This function prepares the deplacement:
 * _nbsteps_: number of steps to execute
 * _speed_: the number of microseconds between each step
Before moving the stepper must know how many steps shall done.

```C++
int Stepper::move(int nbsteps, int hypotenuse, int speed = 4000)
```

This function runs as *turn*, but take the hypotenuse of the movement if there is several stepper motors. All motors should arrive at the same time if the speed is the same.

## Start the deplacement
```C++
void Stepper::start();
```

This function enables the motor and execute the prepared movement.

```C++
Stepper *stepper = new Stepper(10, 11, 12, 14000, 13);

stepper->turn(1000);
stepper->start();
```

## Check the steps
```C++
int Stepper::step();
int Stepper::step(int speed);
```

This function returns the number of steps still awaiting.
For OS without timer (like Arduino) this function must be call as soon as possible.

```C++
Stepper *stepper = new Stepper(10, 11, 12, 14000, 13);

stepper->turn(1000);
stepper->start();
while (stepper->step() > 0);
```

## Stop the deplacement
```C++
void Stepper::stop(int disable);
```

This function stops the motor during the running.

 * **disable** request the library to disable or not the driver after stoping.

## Check the motor's state
```C++
int Stepper::enabled();
```

This function returns true if the driver is enable.

## Current position
```C++
int Stepper::position();
```

This function returns the number of steps from the zero.

## Homing
```C++
void home(int speed);
```

If the endstop is available, the motor runs while the sensor is HIGH.
 * **speed** the number of steps per second.

## Setup the stepper
```C++
enum Setting
{
  MaxPosition,
  Accel,
  MaxSpeed,
  StepsPerMilliMeter,
  MilliMeterMode,
  Movement,
  ForceEnable,
};

int setup(Setting setting);
int setup(Setting setting, int value);
```

This function changes the library running.
 * **MaxPosition** changes the value set with the constructor.
 * **Accel** changes the default accelleration value (default: 10 steps/us).
 * **MaxSpeed** changes the maximum value of the speed and restrict the value from *turn*.
 * **StepsPerMilliMeter** changes the number of steps to do to nove 1mm.
 * **MilliMeterMode** if true, the values for *turn* and *move* are in millimeter.
 * **Movement** changes the speed manager, to execute a linear or circular movement. It accepts *LINEARMOVEMENT* or *CIRCULARMOVEMENT*.
 * **ForceEnable** allows the *stop* function to keep the driver enable after stoping.

# Examples
## Simple usage

```C++
Stepper *stepper[2];

/**
 * create a motor with 3 GPIO (enable:2, dir: 3, step: 4)
 * the maximum of the steps is 14000
 * an endstop sensor is connected on the GPIO 5
 */
stepper[0] = new Stepper(2, 3, 4, 14000, 5);
/**
 * create a second motor
 */
stepper[1] = new Stepper(10, 11, 12, 14000, 13);

stepper[0]->setup(Stepper::StepsPerMilliMeter, 20);
stepper[0]->setup(Stepper::MaxSpeed, 500);
stepper[0]->setup(Stepper::MilliMeterMode, true);

/**
 * search the endstop sensor
 */
stepper[0]->home(500);
stepper[0]->start();
while (stepper[0]->step() > 0);
/** move 5mm before searching the endstop sensor at slow speed */
stepper[0]->turn(5, 10);
stepper[0]->start();
while (stepper[0]->step() > 0);
/** search the endstop sensor again */
stepper[0]->home(10);
stepper[0]->start();
while (stepper[0]->step() > 0);


```

## Arduino examples
### gcode
This example take commands from the serial link. Only one motor is described.  
The current commands are:

 * **G28** : to find the zero on each motor
 * **G0**  : to move to the coordinate from command line: G1 X20000 at fast speed
 * **G1**  : exactly the same as G1 but at the feedrate speed
 * **G90** : set the system to use absolute coordinate
 * **G91** : set the system to use relativ coordinate
 * **G2**  : ....should define a rotation movement
 * **M0**  ; stop the motors
 * **M92** : set the number of steps per millimeter for each motor
 * **M112**: urgent stop
 * **M201**: set the maximum acceleration for each motor
 * **M851**: set the offset of each motor (offset the position of endstop in number of steps)
 * **F...**: set the feedrate (slow speed) for one movement
 * **X...**: set a value for the X axis motor (depends on the command)
 * **Y...**: idem for Y axis
 * **Z...**: idem for Z axis
 * **A...**: idem for A axis (rotation around X axis)
 * **B...**: idem for B axis (rotation around Y axis)
 * **C...**: idem for C axis (rotation around Z axis)
 * **#...**: get/set a value in the table of variables

The current variables are:
 * **#500** : default feedrate speed
 * **#501** : default fast speed
 * **#502** : the ortogonal axis to the working plane (default: Z axis)
 * **#503** : the safeted distance to move on the axis
 * **#504** : set relative or absolute coordonnes (same as G90/G91)
 * **#505** : force the motors to stay enable after moving

Returned strings:  
Each command should return at least one line. The contain depends on the command but should begin by:
 * **ok**  : the command is successful
 * **rs**  : the command should be resended, the system is not ready to accept a new command
 * **err** : an error occured
