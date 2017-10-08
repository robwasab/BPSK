#ifndef __SPECTRUM_ANALYZER__
#define __SPECTRUM_ANALYZER__

#include <fftw3.h>
#include <pthread.h>
#include "../Module/Module.h"
#include "../PlotController/DataSource.h"

class SpectrumAnalyzer : public DataSource, public Module
{
public:
    SpectrumAnalyzer(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            double fs,
            size_t spectrum_size = 1024);
    ~SpectrumAnalyzer();
    Block * process(Block * sig);
    const char * name();
    size_t size();
    AFPoint get_data(size_t index);
    void next();
    AFPoint get_origin();
    AFPoint get_lengths();
    bool valid();
    int get_updateInterval();
private:
    size_t n;
    int update_interval;
    float * freq;
    float * inpu;
    float * outp;
    fftwf_plan fwrd;
    Queue<float> queue;
};

#endif
