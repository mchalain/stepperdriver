
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

#define SAFETED 5

const char motion[6] = { 'X', 'Y', 'Z', 'A', 'B', 'C'};
Stepper *stepper[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

#define FEEDRATE 1
#define RAPIDRATE 2
#define ORTOGONALAXIS 3
#define NBVARIABLES ORTOGONALAXIS
int variables[NBVARIABLES] = {
  1000, 1000, ZM
};
int ortogonalaxis = ZM;

void setup() {
  Serial.begin(11500);
  while (!Serial);
  stepper[XM] = new Stepper(2,3,4,14000,5);
  pinMode(ledPin,OUTPUT);
}

void line(int x, int y, int z, int speed)
{
  if (stepper[XM] != NULL && x != stepper[XM]->position())
  {
    stepper[XM]->setup(Stepper::Movement, LINEARMOVEMENT);
    x = stepper[XM]->turn(x - stepper[XM]->position(), speed);
  }
  if (stepper[YM] != NULL && y != stepper[YM]->position())
  {
    stepper[YM]->setup(Stepper::Movement, LINEARMOVEMENT);
    y = stepper[YM]->turn(y - stepper[YM]->position(), speed);
  }
  if (stepper[ZM] != NULL && z != stepper[ZM]->position())
  {
    stepper[ZM]->setup(Stepper::Movement, LINEARMOVEMENT);
    z = stepper[ZM]->turn(z - stepper[ZM]->position(), speed);
  }
  for (int i = 0; i < NBAXIS; i++)
  {
    if (stepper[i])
      stepper[i]->start();
  }
}
void circle(int xorig, int yorig, int zorig, int diameter, int speed)
{

  if (stepper[XM] != NULL && diameter <= stepper[XM]->position() + stepper[XM]->max())
  {
    stepper[XM]->setup(Stepper::Movement, CIRCULARMOVEMENT);
    stepper[XM]->turn(diameter, speed);
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
  while (cmd.length() > index)
  {
    if (cmd.charAt(index) == option)
      break;
    index++;
  }
  if (cmd.length() > index)
  {
    index++;
    defvalue = cmd.substring(index).toInt();
  }
  return defvalue;
}

void executeGCode(String cmd)
{
  static int relativ = 0;
  int cmdIndex = parseNumber(cmd, 'G', -1);
  switch (cmdIndex)
  {
    case 0:
    case 1:
    {
      int value[NBAXIS] = {0};
      int speed = parseNumber(cmd, 'F', FEEDRATE);
      if (cmdIndex == 0) /** G0 means rapid traverse **/
        speed = RAPIDRATE;
      for (int i = 0; i < NBAXIS; i++)
      {
        value[i] = parseNumber(cmd, motion[i], 0);
        value[i] += (stepper[i] && relativ)? stepper[i]->position():0;
      }
      line(value[XM], value[YM], value[ZM], speed);
    }
    break;
    case 2:
    {
      int speed = parseNumber(cmd, 'F', FEEDRATE);
      int diameter = parseNumber(cmd, 'D', 1000);
      int x = parseNumber(cmd, 'X', 0);
      int y = parseNumber(cmd, 'Y', 0);
      int z = parseNumber(cmd, 'Z', 0);
      circle(x, y, z, diameter, speed);
    }
    break;
    case 90:
      relativ = 0;
    break;
    case 91:
      relativ = 1;
    break;
    case 28:
    {
      /**
       * Up the head before to search the home on the other axis
       */
      if (stepper[variables[ORTOGONALAXIS]])
      {
        stepper[variables[ORTOGONALAXIS]]->setup(Stepper::Movement, LINEARMOVEMENT);
        stepper[variables[ORTOGONALAXIS]]->turn(SAFETED - stepper[XM]->position(), RAPIDRATE);
      }
      for (int i = 0; i < NBAXIS; i++)
      {
        if (stepper[i])
        {
          stepper[i]->home(variables[RAPIDRATE]);
          stepper[i]->start();
          stepper[i]->turn(100, variables[RAPIDRATE]);
          stepper[i]->start();
          stepper[i]->home(variables[FEEDRATE]);
          stepper[i]->start();
        }
      }
    }
    break;
  }
  cmdIndex = parseNumber(cmd, 'M', -1);
  switch (cmdIndex)
  {
    case 0:
    {
      for (int i = 0; i < NBAXIS; i++)
      {
        if (stepper[i])
        {
          stepper[i]->stop();
        }
      }
    }
    break;
    case 92:
    {
      int stepspermilli;
      for (int i = 0; i < NBAXIS; i++)
      {
        stepspermilli = parseNumber(cmd, motion[i], -1);
        stepper[i]->setup(Stepper::StepsPerMilliMeter, stepspermilli);
      }
    }
    break;
  }
}

void loop() {
  if (Serial.available() != 0)
  {
    String cmd = Serial.readString();
    cmd.trim();
    Serial.printf("%s \r\n", cmd.c_str());
    executeGCode(cmd);
  }
  for (int i = 0; i < NBAXIS; i++)
  {
    int ret = 0;
    if (stepper[i])
    {
      ret = stepper[i]->step();
      if (ret == 0)
      {
        if (stepper[i]->enabled())
        {
          Serial.printf("position %c: %d\r\n", motion[i], stepper[i]->position());
        }
        digitalWrite(ledPin,LOW);
      }
      else if (ret > 0)
      {
        digitalWrite(ledPin,HIGH);
      }
      else
      {
        Serial.printf("Stopend\r\n");
        digitalWrite(ledPin,LOW);
      }
    }
  }
}
