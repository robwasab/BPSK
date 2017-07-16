#ifndef __QPSK_H__
#define __QPSK_H__

#include "../Module/Module.h"
#include "CostasLoop.h"
#include "Plottable_CostasLoop.h"
#include "../Filter/BandPass.h"
#include "../switches.h"

#ifdef DEBUG_CONSTELLATION
class QPSK : public Plottable_CostasLoop
#else
class QPSK : public CostasLoop
#endif
{
public:
    QPSK(Memory * memory,
            TransceiverCallback cb,
            void * trans,
            double fs,
            double fc,
            double biqu_fcut = 550.0,
            #ifdef DEBUG_CONSTELLATION
            double loop_fnat = 200.0,
            size_t chunk = 1024);
            #else
            double loop_fnat = 200.0);
            #endif

    const char * name();

    void error_detector(float input,
        double * error,
        double * in_phase,
        double * qu_phase,
        double * lock);

    ~QPSK();

private:
    double vco_offset_2;
    double vco_offset_3;
    double vco_offset_4;

    Biquad_LowPass * lp2;
    Biquad_LowPass * lp4;
    Biquad_LowPass * in_lpf;
    Biquad_LowPass * qu_lpf;

    //override
    Block * process(Block * block);

    BandPass * bp;
};

#endif
