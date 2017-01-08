#ifndef __AUTO_GAIN_H__
#define __AUTO_GAIN_H__

#include "../Module/Module.h"
#include "../CostasLoop/RC_LowPass.h"

class Autogain : public Module
{
public:
    Autogain(Memory * memory, Module * next, double fs);
    ~Autogain();
    const char * name();
    Block * process(Block * sig);

private:
    float peak_tau;
    float lowp_tau;
    float comp_thresh;
    float max_gain;
    float gain;
    //RC_LowPass * peak_cond;
    RC_LowPass * peak;
    RC_LowPass * lowp;
};

#endif
