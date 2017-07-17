#include "FirLowPass.h"
#include "math.h"

#define DEBUG

FirLowPass::FirLowPass(Memory * memory, 
        TransceiverCallback cb, 
        void * transceiver,
        int pwr_of_two, 
        double fp,
        double fs,
        WindowType w_type): 
    PlottableFirFilter(memory, cb, transceiver, (1 << pwr_of_two), 
    filter( (1<<pwr_of_two), fp, 0.0, fs, LOWPASS, w_type) )
{
}

FirLowPass::~FirLowPass()
{
    delete [] coefs;
}
