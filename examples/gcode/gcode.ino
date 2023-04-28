
/**
 * GCode parser for stepperdriver
 * Commandes vailable:
 *  G28 : search the endstop of each axe
 *  G0 X2000 : move the motor of X axe to 2000
 *  G1 X400 : move the motor of X axe to 2000 fast
 *  G2 D3000 : move X Y motors to generate circle diameter 3000
 */
#include "stepperdriver.hpp"

#define ledPin 25

#define NBAXIS 6
#define XM 0
#define YM 1
#define ZM 2
#define AM 3
#define BM 4
#define CM 5

const char motion[6] = { 'X', 'Y', 'Z', 'A', 'B', 'C'};
Stepper *stepper[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

#define FEEDRATE 0
#define RAPIDRATE 1
#define ORTOGONALAXIS 2
#define SAFETED 3
#define ABSOLUTE 4
#define NBVARIABLES ABSOLUTE
int variables[NBVARIABLES + 1] = {
  1000, 1000, ZM, 100, 0
};

// function for sequences
typedef int (*action_f)(int running);
action_f action = NULL;

void setup() {
  Serial.begin(11500);
  while (!Serial);
  stepper[XM] = new Stepper(2,3,4,14000,5);
  pinMode(ledPin,OUTPUT);
}

void line(int coord[NBAXIS], int speed)
{
  int H = 0;
  for (int i = 0; i < 3; i++)
  {
    if (i != variables[ORTOGONALAXIS] && coord[i])
      H += sq(coord[i]);
  }
  if (H)
    H = sqrt(H);
  for (int i = 0; i < 3; i++)
  {
    if (stepper[i] && coord[i] != 0)
    {
      stepper[i]->setup(Stepper::Movement, LINEARMOVEMENT);
      if (i != variables[ORTOGONALAXIS])
        stepper[i]->move(coord[i], H, speed);
      else
        stepper[i]->turn(coord[i], speed);
    }
  }
  for (int i = 0; i < NBAXIS; i++)
  {
    if (stepper[i])
    {
      stepper[i]->start();
    }
  }
}

void circle(int coord[NBAXIS], int diameter, int speed)
{
  for (int i = 0; i < 3; i++)
  {
    if (stepper[i] && coord[i] != 0)
    {
      if (i != variables[ORTOGONALAXIS])
        stepper[i]->setup(Stepper::Movement, CIRCULARMOVEMENT);
      else
        stepper[i]->setup(Stepper::Movement, LINEARMOVEMENT);
      stepper[i]->turn(diameter, speed);
    }
  }
  for (int i = 0; i < NBAXIS; i++)
  {
    if (stepper[i])
      stepper[i]->start();
  }
}

int parseNumber(String cmd, char option, int defvalue)
{
  int index = 0;
  index = cmd.indexOf(option);
  if (index < 0)
    return defvalue;
  if (cmd.length() > index)
  {
    index++;
    defvalue = cmd.substring(index).toInt();
  }
  return defvalue;
}

int unique_action(int running)
{
  return --running;
}

int home_action(int running)
{
  int axis = NBAXIS - ((running + 2) / 3);
  switch (running)
  {
    case 15 + 1:
    case 12 + 1:
    case 9 + 1:
    case 6 + 1:
    case 3 + 1:
    case 1:
    {
      if (stepper[axis])
      {
        stepper[axis]->home(variables[FEEDRATE]);
        stepper[axis]->start();
      }
    }
    break;
    case 15 + 2:
    case 12 + 2:
    case 9 + 2:
    case 6 + 2:
    case 3 + 2:
    case 2:
    {
      if (stepper[axis])
      {
        stepper[axis]->turn(variables[SAFETED], variables[RAPIDRATE]);
        stepper[axis]->start();
      }
    }
    break;
    case 15 + 3:
    case 12 + 3:
    case 9 + 3:
    case 6 + 3:
    case 3 + 3:
    case 3:
    {
      if (stepper[axis])
      {
        stepper[axis]->home(variables[RAPIDRATE]);
        stepper[axis]->start();
      }
    }
    break;
  }
  return --running;
}

int executeGCode(String cmd, int running)
{
  int coord[NBAXIS] = {0, 0, 0, 0, 0, 0};
  int cmdIndex;

  cmdIndex = parseNumber(cmd, 'M', -1);
  if (cmdIndex == 112)
  {
    for (int i = 0; i < NBAXIS; i++)
    {
      if (stepper[i])
      {
        stepper[i]->stop();
      }
    }
    return 0;
  }
  if (running)
    return -1;
  switch (cmdIndex)
  {
    case 0:
    {
      for (int i = 0; i < NBAXIS; i++)
      {
        if (stepper[i])
        {
          stepper[i]->stop(true);
        }
      }
    }
    break;
    case 92:
    {
      int stepspermilli;
      for (int i = 0; i < NBAXIS; i++)
      {
        if (stepper[i])
        {
          stepspermilli = parseNumber(cmd, motion[i], -1);
          stepper[i]->setup(Stepper::StepsPerMilliMeter, stepspermilli);
          stepper[i]->setup(Stepper::MilliMeterMode, 1);
        }
      }
    }
    break;
    case 201:
    {
      int accel;
      for (int i = 0; i < NBAXIS; i++)
      {
        accel = parseNumber(cmd, motion[i], -1);
        stepper[i]->setup(Stepper::Accel, accel);
      }
    }
  }
  /**
   * Parse X..., Y..., Z..., A..., B..., C...
   */
  for (int i = 0; i < NBAXIS; i++)
  {
    coord[i] = parseNumber(cmd, motion[i], 0);
    /**
     * StepperDriver library uses only Relativ coordonnes
     */
    if (!variables[ABSOLUTE] && coord[i] && stepper[i])
      coord[i] -= stepper[i]->position();
  }
  int speed = parseNumber(cmd, 'F', variables[FEEDRATE]);
  cmdIndex = parseNumber(cmd, 'G', -1);
  switch (cmdIndex)
  {
    case 0:
      /** G0 means rapid traverse **/
      speed = variables[RAPIDRATE];
    case 1:
    {
      line(coord, speed);
      action = unique_action;
      running = 1;
    }
    break;
    case 2:
    {
      int diameter = parseNumber(cmd, 'D', 1000);
      circle(coord, diameter, speed);
      action = unique_action;
      running = 1;
    }
    break;
    case 90:
      variables[ABSOLUTE] = 1;
    break;
    case 91:
      variables[ABSOLUTE] = 0;
    break;
    case 28:
    {
      /**
       * Up the head before to search the home on the other axis
       */
      if (stepper[variables[ORTOGONALAXIS]])
      {
        stepper[variables[ORTOGONALAXIS]]->setup(Stepper::Movement, LINEARMOVEMENT);
        stepper[variables[ORTOGONALAXIS]]->turn(variables[SAFETED], variables[RAPIDRATE]);
      }
      action = home_action;
      running = 19;
    }
    break;
  }
  cmdIndex = parseNumber(cmd, '#', -1);
  if (cmdIndex > 500)
  {
    int value = parseNumber(cmd, '=', 0x7FFF);
    if (value != 0x7FFF)
      variables[cmdIndex - 500] = value;
    else
      Serial.printf("#%d = %d\r\n", 500+cmdIndex, variables[cmdIndex - 500]);
  }
  return running;
}

void loop()
{
  static int running = 0;
  if (Serial.available() != 0)
  {
    String cmd = Serial.readString();
    cmd.trim();
    running = executeGCode(cmd, running);
    if (running < 0)
    {
      Serial.printf("rs\r\n");
    }
  }
  if (running == 0 || action == NULL)
  {
    running == 0;
    delay(1000);
    return;
  }
  digitalWrite(ledPin,HIGH);
  int ret = 0;
  for (int i = 0; i < NBAXIS; i++)
  {
    if (stepper[i])
    {
      int tmp = stepper[i]->step();
      if (tmp > 0)
        ret += tmp;
      else if (tmp < 0)
        Serial.printf("err endstop\r\n");
    }
  }
  if (ret == 0)
  {
      running = action(running);
  }
  if (running == 0)
  {
    Serial.printf("ok ");
    for (int i = 0; i < NBAXIS; i++)
    {
      if (stepper[i] && stepper[i]->enabled())
      {
        Serial.printf("%c: %d ", motion[i], stepper[i]->position());
      }
    }
    Serial.printf("\r\n");
    digitalWrite(ledPin,LOW);
    action = NULL;
  }
}
