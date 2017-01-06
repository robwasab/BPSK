#ifndef __SPECTRUM_ANALYZER__
#define __SPECTRUM_ANALYZER__

#include <fftw3.h>
#include "../PlotController/DataSource.h"

class SpectrumAnalyzer : public DataSource
{
public:
    SpectrumAnalyzer(int n, double fs);
    size_t size();
    Point get_data(size_t index);
    void next();
    Point get_origin();
    Point get_lengths();
    bool valid();
    const char * name();
private:
    int n;
    float * data;
    float * freqs;
};

#endif
