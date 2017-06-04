#ifndef __QPSK_H__
#define __QPSK_H__

#include "../Module/Module.h"
#include "CostasLoop.h"

class QPSK : public CostasLoop
{
public:
    QPSK(Memory * memory,
            TransceiverCallback cb,
            void * trans,
            double fs,
            double fc,
            double biqu_fcut = 550.0,
            double loop_fnat = 200.0);


    const char * name();

    void error_detector(float input,
        double * error,
        double * in_phase,
        double * qu_phase,
        double * lock);

private:
    double vco_offset_2;
    double vco_offset_3;
    double vco_offset_4;

    Biquad_LowPass * lp2;
    Biquad_LowPass * lp4;

    //override
    Block * process(Block * block);
};

#endif
