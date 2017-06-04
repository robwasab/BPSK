#ifndef __SECOND_ORDER_PLL_H__
#define __SECOND_ORDER_PLL_H__

#include "../Module/Module.h"
#include "Integrator.h"
#include <math.h>

class SecondOrderPLL : public Module 
{
public:
    SecondOrderPLL(Memory * memory, 
            TransceiverCallback transceiver_cb,
            void * transceiver,
            double fs, 
            double fc, 
            double loop_fnat, 
            double loop_zeta=0.707106):
        Module(memory, transceiver_cb, transceiver),
        fs(fs),
        fc(fc),
        loop_fnat(loop_fnat),
        loop_zeta(loop_zeta)
    {
        // The variable G and a so that the loop achieves the above responses
        G = 4.0 * M_PI * loop_zeta * loop_fnat;
        a = loop_fnat * M_PI / loop_zeta;

        inc = 2.0 * M_PI * this->fc;
        vco = new Integrator(fs);
        amp = new Integrator(fs);
    }

    ~SecondOrderPLL()
    {
        delete vco;
        delete amp;
    }

    void reset() {
        vco->reset();
        amp->reset();
    }

    virtual void work(float input, 
            float * in_phase, 
            float * qu_phase,
            float * freq_ptr, 
            float * lock_ptr, 
            float * error) = 0;

    virtual const char * name() = 0;
    virtual Block * process(Block * in) = 0;

    virtual void error_detector(float input, 
            double * error, 
            double * in_phase, 
            double * qu_phase,
            double * lock) = 0;
protected:
    double fs;
    double fc;
    double loop_fnat;
    double loop_zeta;

    // Phase Integrator for the vco
    Integrator * vco;

    // loop variables from pll theory
    // book: basic simulation models of phase tracking devices using matlab
    // author: william tranter, ratchaneekorn thamvichai, tamal bose
    // section 4.1 basic models fro phase-locked loops, pg. 47, figure 4.2
    // appendix b.2 the loop filter for the perfect second-order phase-locked 
    // loop, pg. 100-101
    double G;

    double a;

    // Loop Integrator
    Integrator * amp;

    // the amount of phase to increment the vco each iteration
    double inc;
};

#endif
