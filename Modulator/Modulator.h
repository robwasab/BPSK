#ifndef __MODULATOR_H__
#define __MODULATOR_H__

#include "../Module/Module.h"
#include "../CostasLoop/Integrator.h"

class Modulator : public Module
{
public:
    Modulator(Memory * memory, Module * next, double fs, double fc);
    ~Modulator();

    const char * name();
    Block * process(Block * msg);
private:
    double inc;
    double phase;
    Integrator * vco;
};

#endif
