
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
#define NORMALSPEED 1000

Stepper *stepper[4] = {NULL, NULL, NULL, NULL};

void setup() {
  Serial.begin(11500);
  while (!Serial);
  stepper[0] = new Stepper(2,3,4,14000,5);
  pinMode(ledPin,OUTPUT);
}

void line(int x, int y, int z, int speed)
{
  if (stepper[0] != NULL && x != stepper[0]->position())
  {
    stepper[0]->setup(Stepper::Movement, LINEARMOVEMENT);
    x = stepper[0]->turn(x - stepper[0]->position(), speed);
  }
  if (stepper[1] != NULL && y != stepper[1]->position())
  {
    stepper[1]->setup(Stepper::Movement, LINEARMOVEMENT);
    y = stepper[1]->turn(y - stepper[1]->position(), speed);
  }
  if (stepper[2] != NULL && z != stepper[2]->position())
  {
    stepper[2]->setup(Stepper::Movement, LINEARMOVEMENT);
    z = stepper[2]->turn(z - stepper[2]->position(), speed);
  }
  for (int i = 0; i < 4; i++)
  {
    if (stepper[i])
      stepper[i]->start();
  }
}
void circle(int xorig, int yorig, int zorig, int diameter, int speed)
{

  if (stepper[0] != NULL && diameter <= stepper[0]->position() + stepper[0]->max())
  {
    stepper[0]->setup(Stepper::Movement, CIRCULARMOVEMENT);
    stepper[0]->turn(diameter, speed);
  }
  for (int i = 0; i < 4; i++)
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
      int speed = parseNumber(cmd, 'F', NORMALSPEED);
      int x = parseNumber(cmd, 'X', 0);
      x += (stepper[0] && relativ)? stepper[0]->position():0;
      int y = parseNumber(cmd, 'Y', 0);
      y += (stepper[1] && relativ)? stepper[1]->position():0;
      int z = parseNumber(cmd, 'Z', 0);
      z += (stepper[2] && relativ)? stepper[2]->position():0;
      line(x, y, z, speed);
    }
    break;
    case 2:
    {
      int speed = parseNumber(cmd, 'F', NORMALSPEED);
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
      if (stepper[0])
      {
        stepper[0]->home();
        stepper[0]->start();
      }
      if (stepper[1])
      {
        stepper[1]->home();
        stepper[1]->start();
      }
      if (stepper[2])
      {
        stepper[2]->home();
        stepper[2]->start();
      }
    }
    break;
  }
  cmdIndex = parseNumber(cmd, 'M', -1);
  switch (cmdIndex)
  {
    case 0:
    {
      if (stepper[0])
      {
        stepper[0]->stop();
      }
      if (stepper[1])
      {
        stepper[1]->stop();
      }
      if (stepper[2])
      {
        stepper[2]->stop();
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
  for (int i = 0; i < 4; i++)
  {
    int ret = 0;
    if (stepper[i])
    {
      ret = stepper[i]->step();
      if (ret == 0)
      {
        if (stepper[i]->enabled())
        {
          Serial.printf("position %d: %d\r\n", i, stepper[i]->position());
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
