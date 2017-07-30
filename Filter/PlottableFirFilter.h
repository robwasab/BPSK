#ifndef __PLOTTABLE_FIR_FILTER_H__
#define __PLOTTABLE_FIR_FILTER_H__

#include "FirFilter.h"
#include "../PlotController/DataSource.h"

class PlottableFirFilter : public FirFilter, public DataSource
{
public:
    PlottableFirFilter(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver,
            size_t n, 
            const double coefs[]);

    /* DataSource Methods */
    size_t size();
    Point get_data(size_t index);
    void next();
    Point get_origin();
    Point get_lengths();
    bool valid();
    int get_updateInterval();
    const char * name();

private:
    float max;
    float min;
    bool mvalid;
};

#endif
