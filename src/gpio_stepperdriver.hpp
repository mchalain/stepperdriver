#ifndef __GPIO_STEPPERDRIVER_HPP__
#define __GPIO_STEPPERDRIVER_HPP__

class GeneralIO
{
  public:
    virtual bool value() = 0;
};

class GeneralInput : public GeneralIO
{
  public:
    virtual bool value() = 0;
    static GeneralInput *makeGeneralInput(int chip, int number, bool inverted = false);
};

class GeneralOutput : public GeneralIO
{
  public:
    virtual bool value() = 0;
    virtual void value(bool change) = 0;
    static GeneralOutput *makeGeneralOutput(int chip, int number);
};

#endif
