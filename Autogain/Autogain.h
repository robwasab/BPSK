#ifndef __AUTO_GAIN_H__
#define __AUTO_GAIN_H__

#include "../Module/Module.h"
#include "../CostasLoop/RC_LowPass.h"
#include <math.h>

class AutogainParams
{
public: 
    AutogainParams(double max_gain, double thresh, double fs):
        max_gain(max_gain),
        thresh(thresh)
    {
        peak_rc = new RC_LowPass(0.005, fs);
        cond_rc = new RC_LowPass(0.0075, fs);
        comp_rc = new RC_LowPass(0.5, fs);
        gain = 1.0;
    }

    ~AutogainParams()
    {
        delete peak_rc;
        delete cond_rc;
        delete comp_rc;
    }

    double work(double sig)
    {
        double rect; 
        double peak_out;
        double cond_out;
        double comp_out;
        double outp;
        
        outp = sig * gain;

        rect = fabs(outp);

        if (rect > peak_rc->value())
        {
            peak_rc->last = rect;
            peak_out = rect;
        }
        else
        {
            peak_out = peak_rc->work(0.0);
        }

        cond_out = cond_rc->work(peak_out);

        if (cond_out < thresh)
        {
            comp_out = max_gain;
        }
        else
        {
            comp_out = 0.0;
        }

        gain = 1.0 + comp_rc->work(comp_out);

        return outp;
    }

private:
    double max_gain;
    double thresh;
    double gain;
    RC_LowPass * peak_rc;
    RC_LowPass * cond_rc;
    RC_LowPass * comp_rc;
};

class Autogain : public Module
{
public:
    Autogain(Memory * memory, Module * next, double fs);
    ~Autogain();
    const char * name();
    Block * process(Block * sig);

private:
    AutogainParams autogain_c;
    AutogainParams autogain_b;
    AutogainParams autogain_a;
};

#endif
