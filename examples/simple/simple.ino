#include "stepperdriver.hpp"

#define ledPin 25

Stepper *stepper;

void setup()
{
  stepper = new Stepper(2,3,4,40000);
  pinMode(ledPin,OUTPUT);
  Serial.begin(11500);
  while (!Serial);
}

int sens = 1;
void loop()
{
  if (!stepper->enabled())
  {
    stepper->stop();
    digitalWrite(ledPin,LOW);
    delay(1000);
    stepper->turn(sens * 4000);
    sens *= -1;
    stepper->start();
    digitalWrite(ledPin,HIGH);
  }
  stepper->step();
}
