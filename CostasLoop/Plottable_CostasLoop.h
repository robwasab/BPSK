#ifndef __PLOTTABLE_COSTASLOOP_H__
#define __PLOTTABLE_COSTASLOOP_H__

#include "CostasLoop.h"
#include "../PlotController/DataSource.h"
#include "../PlotController/PlotController.h"
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
            double lock_thesh = 0.5);

    /* DataSource Methods */
    size_t size();
    AFPoint get_data(size_t index);
    void next();
    AFPoint get_origin();
    AFPoint get_lengths();
    bool valid();
    int get_updateInterval();
    const char * name();
    const char * xlabel()
    {
        static const char text[] = "Real";
        return text;
    }
    const char * ylabel()
    {
        static const char text[] = "Imaginary";
        return text;
    }

    /* CostasLoop Methods */
    virtual void work(float input, 
            float * in_phase, 
            float * qu_phase,
            float * freq_ptr, 
            float * lock_ptr, 
            float * error);

    void dispatch(RadioMsg * msg);

    ~Plottable_CostasLoop();

private:
    int queue_full_warnings;
    size_t frame_size;
    int update_interval;
    Queue<float> in_phase_queue;
    Queue<float> qu_phase_queue;
    float * in_phase_memory;
    float * qu_phase_memory;
};
#endif
