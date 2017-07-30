#include "Plottable_CostasLoop.h"
#include "../switches.h"
#include <string.h>

Plottable_CostasLoop::Plottable_CostasLoop(
            Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver, 
            double fs,
            double fc, 
            double biqu_fcut,
            double loop_fnat,
            double lock_threshold,
            size_t chunk):
    CostasLoop(memory, 
            cb, 
            transceiver, 
            fs, 
            fc, 
            IN_PHASE_SIGNAL,
            biqu_fcut, 
            loop_fnat, 
            lock_threshold),
    chunk(chunk),
    in_phase_queue((1 << 18)),
    qu_phase_queue((1 << 18))
{
    update_interval = (int) round(chunk/fs);
    in_phase_memory = new float[chunk];
    qu_phase_memory = new float[chunk];
    memset(in_phase_memory, 0, sizeof(float) * chunk);
    memset(qu_phase_memory, 0, sizeof(float) * chunk);
}

int Plottable_CostasLoop::get_updateInterval()
{
    return update_interval;
}

Plottable_CostasLoop::~Plottable_CostasLoop()
{
    delete [] in_phase_memory;
    delete [] qu_phase_memory;
}

size_t Plottable_CostasLoop::size()
{
    return chunk;
}

Point Plottable_CostasLoop::get_data(size_t index)
{
    Point p;
    p.x = in_phase_memory[index];
    p.y = qu_phase_memory[index];
    return p;
}

void Plottable_CostasLoop::next()
{

}

Point Plottable_CostasLoop::get_origin()
{
    Point p;
    p.x = -1.5;
    p.y = -1.5;
    return p;
}

Point Plottable_CostasLoop::get_lengths()
{
    Point p;
    p.x = 3.0;
    p.y = 3.0;
    return p;
}

bool Plottable_CostasLoop::valid()
{
    bool valid = !(in_phase_queue.size() >= chunk);

    if (!valid)
    {
        (void) in_phase_queue.get(in_phase_memory, chunk);
        (void) qu_phase_queue.get(qu_phase_memory, chunk);
        return false;
    }
    return true;
}

void Plottable_CostasLoop::work(float val, 
            float * in_phase, 
            float * qu_phase,
            float * freq_ptr, 
            float * lock_ptr, 
            float * error)
{
    CostasLoop::work(val, in_phase, qu_phase, freq_ptr, lock_ptr, error);
    
    bool in_res = true, qu_res = true;

    in_res = in_phase_queue.add(*in_phase);
    qu_res = qu_phase_queue.add(*qu_phase);

    if (!in_res & qu_res)
    {
        ERROR("In-Phase Queue Full!\n");
    }
    else if (in_res & !qu_res)
    {
        ERROR("Qu-Phase Queue FULL!\n");
    }
    else if (!in_res & !qu_res)
    {
        ERROR("Both In-Phase and Qu-Phase Queues are FULL!\n");
    }
}

const char * Plottable_CostasLoop::name()
{
    return CostasLoop::name();
}

