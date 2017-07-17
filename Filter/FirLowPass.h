#ifndef __FIR_LOWPASS_H__
#define __FIR_LOWPASS_H__

#include "PlottableFirFilter.h"
#include "FirFilter.h"
#include "math.h"

class FirLowPass : public PlottableFirFilter
{
public:
    FirLowPass(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver,
            int power_of_two,
            double fp,
            double fs = 44.1E3,
            WindowType w_type = BLACKMAN);

    ~FirLowPass();
};

#endif
