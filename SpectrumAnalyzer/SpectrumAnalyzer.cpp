#include <math.h>
#include <fftw3.h>
#include <string.h>
#include "SpectrumAnalyzer.h"

const char __SPECTRUM_ANALYZER_NAME__[] = "SpectrumAnalyzer";

const char * SpectrumAnalyzer::name() {
    return __SPECTRUM_ANALYZER_NAME__;
}

#define IN_PLACE

SpectrumAnalyzer::SpectrumAnalyzer(Memory * memory, 
        Module * next, 
        double fs,
        size_t spectrum_size):
    Module(memory, next),
    n(spectrum_size),
    queue((1 << 19))
{
    freq = (float *) fftwf_malloc(sizeof(float) * n/2+1);
    inpu = (float *) fftwf_malloc(sizeof(float) * n);
#ifdef IN_PLACE
    outp = inpu;
#else
    outp = (float *) fftwf_malloc(sizeof(float) * n);
#endif

    fwrd = fftwf_plan_r2r_1d
        (n, inpu, outp, FFTW_R2HC, FFTW_MEASURE);

    for (size_t k = 0; k < n/2+1; ++k) 
    {
        freq[k] = fs * ((float) k) / ((float) n);
        //printf("freq[%d] = %.3f\n", k, freq[k]);
    }

    update_interval = (int) round(1000.0*n/fs);
    memset(inpu, 0, sizeof(float) * n);
    memset(outp, 0, sizeof(float) * n);
}

int SpectrumAnalyzer::get_updateInterval()
{
    return update_interval;
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    fftwf_free(freq);
    fftwf_free(inpu);
#ifndef IN_PLACE
    fftwf_free(outp);
#endif
}

Block * SpectrumAnalyzer::process(Block * in)
{
    static char errors[][25] = 
    {
        {"No Error"},
        {"Queue full"}
    };
    int error = 0;
    float ** in_iter = in->get_iterator();
    in->reset();
 
    do
    {
        if (!queue.add(**in_iter)) 
        {
            error = 1;
            goto fail;
        }
    } while(in->next());

    return in;

fail:
    in->free();
    ERROR("%s\n", errors[error]);
    return NULL;
}

size_t SpectrumAnalyzer::size() 
{
    return n/2;
}

#define sq(x) (x * x)

Point SpectrumAnalyzer::get_data(size_t index) 
{
    Point p;
    p.x = freq[index];

    if (index == 0 || index == n/2) {
        p.y = outp[index];
    }
    else {
        p.y = sqrt( sq(outp[index]) + sq(outp[n-index]) );
    }

    p.y = 10.0 * log10(p.y);
    return p;
}

void SpectrumAnalyzer::next() 
{
}

Point SpectrumAnalyzer::get_origin() 
{
    Point p;
    p.x = 0.0;
    p.y = -60;
    return p;
}

Point SpectrumAnalyzer::get_lengths() 
{
    Point p;
    p.x = freq[n/2];
    p.y = 60;
    return p;
}

bool SpectrumAnalyzer::valid() 
{
    bool valid = !(queue.size() >= n);

    if (!valid) 
    {
        (void) queue.get(inpu, n); 
        fftwf_execute(fwrd);
        for (size_t k = 0; k < n; ++k)
        {
            outp[k] /= (float) n/2.0;
        }
        return false;
    }
    return true;
}
