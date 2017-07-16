#ifndef __PLOTTABLE_COSTASLOOP_H__
#define __PLOTTABLE_COSTASLOOP_H__

#include "CostasLoop.h"
#include "../PlotController/DataSource.h"
#include "../Queue/Queue.h"

class Plottable_CostasLoop : public CostasLoop, public DataSource
{

public:
    Plottable_CostasLoop(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            double fs, 
            double fc, 
            double biqu_fcut = 600.0,
            double loop_fnat = 200.0,
            double lock_thesh = 0.5,
            size_t chunk = 1024);

    /* DataSource Methods */
    size_t size();
    Point get_data(size_t index);
    void next();
    Point get_origin();
    Point get_lengths();
    bool valid();
    int get_updateInterval();

    /* CostasLoop Methods */
    virtual void work(float input, 
            float * in_phase, 
            float * qu_phase,
            float * freq_ptr, 
            float * lock_ptr, 
            float * error);

    ~Plottable_CostasLoop();

private:
    size_t chunk;
    int update_interval;
    Queue<float> in_phase_queue;
    Queue<float> qu_phase_queue;
    float * in_phase_memory;
    float * qu_phase_memory;
};
#endif
