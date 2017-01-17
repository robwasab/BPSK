#ifndef __COSTAS_LOOP_H__
#define __COSTAS_LOOP_H__

#include "../Module/Module.h"
#include "Integrator.h"
#include "Biquad_LowPass.h"
#include "RC_LowPass.h"
#include "LockDetector.h"

enum SignalType
{
    IN_PHASE_SIGNAL,
    FREQUENCY_EST_SIGNAL,
    LOCK_SIGNAL,
    ERROR_SIGNAL
};

class CostasLoop : public Module
{
public:
    CostasLoop(Memory * memory, 
            Module * next, 
            double fs, 
            double fc, 
            SignalType type = IN_PHASE_SIGNAL);
    ~CostasLoop();
    Block * process(Block * block);
    float work(float input, float * freq_ptr, float * lock_ptr, float * error);
    const char * name();
    void reset();

private:

    // sampling frequency
    double fs;

    // center frequency
    double fc;

    SignalType type;

    // Loop variables from PLL theory
    // Book: Basic Simulation Models of Phase Tracking devices Using MATLAB
    // Author: William Tranter, Ratchaneekorn Thamvichai, Tamal Bose
    // Section 4.1 Basic Models fro Phase-Locked Loops, Pg. 47, figure 4.2
    // Appendix B.2 The Loop Filter for the Perfect Second-Order Phase-Locked 
    // Loop, Pg. 100-101
    double G;
    double a;

    // Loop Integrator
    Integrator * amp;

    // the amount of phase to increment the vco each iteration
    double inc;

    // Phase Integrator for the vco
    Integrator * vco;
    //PeriodicIntegrator * vco;

    // In Phase LPF
    Biquad_LowPass * ilp;

    // Quadrature Phase LPF
    Biquad_LowPass * qlp;

    
    // Filter to help measure the freqruency
    Biquad_LowPass * freq_filter;

    // Constant for measuring frequency
    double HZ_PER_RAD;

    // To measure frequency you need the delta between new and old phase
    double last_vco_phase;


    // LockDetector multiplies the downconverted in and quadrature 
    // phase signals together.
    // If they are below a very low number, the loop is locked
    LockDetector * lock_detector;

    // RC circuit that charges up when lock is true
    RC_LowPass * lock_rc;

};
#endif
