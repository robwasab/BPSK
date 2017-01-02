#include "BPSKDecoder.h"
#include <math.h>

BPSKDecoder::BPSKDecoder(Memory * memory, 
        Module * next,
        float fs, 
        float fc,
        int cycles_per_bit, 
        bool first_bit):
    Module(memory, next)
{
    // the time difference between two sampling rates is
    // exact sampling rate fs, ts
    // integer sampling rate fk, tk
    // the time at sample n is
    // ts*n, tk*n
    // the time difference is:
    // ts*n - tk*n = n*(ts-tk)
    // the maximum allowable drift is half a period ts/2
    // the number of samples at which the maximum drift occurs is
    // ts/2 = n*|ts-tk|
    // n = ts/2/|ts-tk|
    // n = ts/(2*|ts-tk|)
    // n = 0.5/|1-tk/ts|

    // ts = 1/fc * cycles_per_bit = cycles_per_bit/fc
    // tk = samples_per_bit / fs
    // tk/ts = samples_per_bit/cycles_per_bit*fc/fs

    float samples_per_bit;
    float tk_ts;
    float n;

    samples_per_bit = round(fs/fc*cycles_per_bit);

    tk_ts = samples_per_bit/((float)cycles_per_bit)*fc/fs;

    if (fabs(1 - tk_ts) > 1E-6) {
        n = 0.5/fabs(1 - tk_ts);
        BLINK;
        YELLOW;
        printf("Sampling clock drift warning! n = %.3f\n", n);
        printf("It is recommended you change fc\n");
        ENDC;
    }
    sample_period = (int) samples_per_bit;
}

const char __name__[] = "BPSKDecoder";

const char * BPSKDecoder::name()
{
    return __name__;
}

Block * BPSKDecoder::process(Block * b)
{
    return NULL;
}
