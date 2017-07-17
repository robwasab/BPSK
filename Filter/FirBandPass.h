#ifndef __FIR_BANDPASS_H__
#define __FIR_BANDPASS_H__

#include "PlottableFirFilter.h"
#include "math.h"

class FirBandPass : public PlottableFirFilter
{
public:
    FirBandPass(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver,
            double fs,
            double fc,
            double bw,
            int pwr_of_two,
            WindowType w_type = BLACKMAN):
    PlottableFirFilter(memory, cb, transceiver, (1 << pwr_of_two), 
    filter( (1<<pwr_of_two), bw/2.0, fc, fs, BANDPASS, w_type) )
    {
    }

    ~FirBandPass()
    {
        delete [] coefs;
    }
};

#endif
