#ifndef __COSTAS_LOOP_H__
#define __COSTAS_LOOP_H__

#include "../Module/Module.h"
#include "SecondOrderPLL.h"
#include "Biquad_LowPass.h"
#include "RC_LowPass.h"
#include "LockDetector.h"

enum SignalType
{
    IN_PHASE_SIGNAL,
    QU_PHASE_SIGNAL,
    FREQUENCY_EST_SIGNAL,
    LOCK_SIGNAL,
    ERROR_SIGNAL
};

class CostasLoop : public SecondOrderPLL
{
public:
    CostasLoop(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            double fs, 
            double fc, 
            SignalType type = IN_PHASE_SIGNAL,
            double biqu_fcut = 600.0,
            double loop_fnat = 200.0,
            double lock_thesh = 0.5);

    ~CostasLoop();
    virtual void work(float input, 
            float * in_phase, 
            float * qu_phase,
            float * freq_ptr, 
            float * lock_ptr, 
            float * error);
    const char * name();
    void reset();

    void error_detector(float input, 
            double * error, 
            double * in_phase, 
            double * qu_phase,
            double * lock);

protected:
    // Lock detector threshold
    double lock_thresh;

    // In Phase LPF
    Biquad_LowPass * ilp;

    // Quadrature Phase LPF
    Biquad_LowPass * qlp;

    // LockDetector multiplies the downconverted in and quadrature 
    // phase signals together.
    // If they are below a very low number, the loop is locked
    LockDetector * lock_detector;

private:
    Block * process(Block * block);
    SignalType type;
    
    // Filter to help measure the freqruency
    Biquad_LowPass * freq_filter;

    // Constant for measuring frequency
    double HZ_PER_RAD;

    // To measure frequency you need the delta between new and old phase
    double last_vco_phase;

    // RC circuit that charges up when lock is true
    RC_LowPass * lock_rc;
};
#endif
