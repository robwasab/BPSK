#ifndef __FIR_HIGHPASS_H__
#define __FIR_HIGHPASS_H__

#include "PlottableFirFilter.h"
#include "math.h"

class FirHighPass : public PlottableFirFilter
{
public:
    FirHighPass(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver,
            int pwr_of_two,
            double fp,
            double fs = 44.1E3,
            WindowType w_type = BLACKMAN):
    PlottableFirFilter(memory, cb, transceiver, (1 << pwr_of_two), 
    filter( (1<<pwr_of_two), fp, 0.0, fs, HIGHPASS, w_type) )
    {
    }

    ~FirHighPass()
    {
        delete [] coefs;
    }
};

#endif
