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
Stepper::Stepper(int en, int step, int dir, unsigned int max, int end = -1, int enState = LOW);
```

 * _en_: GPIO number to enable the motor
 * _step_: GPIO number to drive a step on the motor
 * _dir_: GPIO number to drive the direction of the motor
 * _max_: the maximum numbers of steps for moving
 * _end_: GPIO number of the end sensor
 * _enState_: the level on the _en_ GPIO to enable the motor

## Prepare the deplacement
```C++
int Stepper::turn(int nbsteps, int speed = 1000)
```

This function prepares the deplacement:
 * _nbsteps_: number of steps to execute
 * _speed_: the number of microseconds between each step

## Start the deplacement
```C++
void Stepper::start()
```

This function enables the motor.

## Check the steps
```C++
int Stepper::step()
int Stepper::step(int speed)
```

This function returns the number of steps still awaiting.
For OS without timer (like Arduino) this function must be call as soon as possible.

## Stop the deplacement
```C++
void Stepper::stop()
```

This function stops the motor during the running.

## Check the motor's state
```C++
int Stepper::enabled()
```

This function returns true if the driver is enable.

## Current position
```C++
int Stepper::position()
```

This function returns the number of steps from the zero.

## Homing
```C++
void home()
```

If the endstop is available, the motor runs while the sensor is HIGH.

# Examples
## gcode
This example take commands from the serial link. Only one motor is described.  
The current commands are:

 * **G28** : to find the zero on each motor
 * **G0**  : to move to the coordinate from command line: G1 X20000
 * **G1**  : exactly the same as G1
 * **G90** : set the system to use absolute coordinate
 * **G91** : set the system to use relativ coordinate
 * **M0**  : to stop all motor during movement
 * **G2**  : ....should define a rotation movement
