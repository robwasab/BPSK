#ifndef __PLOTTABLE_AUTOGAIN_H__
#define __PLOTTABLE_AUTOGAIN_H__
#include "Autogain.h"
#include "../PlotController/DataSource.h"
#include "../PlotController/PlotController.h"
#include "../Queue/Queue.h"

class PlottableAutogain : public Autogain, public DataSource
{
public:
    PlottableAutogain(
            Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver, 
            double fs, 
            size_t chunk=1024);

    /* DataSource Methods */
    size_t size();
    Point get_data(size_t index);
    void next();
    Point get_origin();
    Point get_lengths();
    bool valid();
    int get_updateInterval();
    const char * name();

    /* Autogain Methods */
    float work(float val);
    void dispatch(RadioMsg * msg);

    ~PlottableAutogain();

private:
    size_t chunk;
    int update_interval;
    Queue<float> queue;
    float * plot_memory;
};
#endif
