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
            double lock_threshold):
    CostasLoop(memory, 
            cb, 
            transceiver, 
            fs, 
            fc, 
            IN_PHASE_SIGNAL,
            biqu_fcut, 
            loop_fnat, 
            lock_threshold),
    frame_size(FRAME_SIZE(fs)),
    in_phase_queue(BUFFER_SIZE(fs)),
    qu_phase_queue(BUFFER_SIZE(fs))
{
    queue_full_warnings = 0;
    update_interval = UPDATE_INTERVAL_MS;
    in_phase_memory = new float[frame_size];
    qu_phase_memory = new float[frame_size];
    memset(in_phase_memory, 0, sizeof(float) * frame_size);
    memset(qu_phase_memory, 0, sizeof(float) * frame_size);
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
    return frame_size;
}

AFPoint Plottable_CostasLoop::get_data(size_t index)
{
    AFPoint p;
    p.x = in_phase_memory[index];
    p.y = qu_phase_memory[index];
    return p;
}

void Plottable_CostasLoop::next()
{

}

AFPoint Plottable_CostasLoop::get_origin()
{
    AFPoint p;
    p.x = -1.5;
    p.y = -1.5;
    return p;
}

AFPoint Plottable_CostasLoop::get_lengths()
{
    AFPoint p;
    p.x = 3.0;
    p.y = 3.0;
    return p;
}

bool Plottable_CostasLoop::valid()
{
    bool data_to_plot = in_phase_queue.size() >= frame_size;

    if (data_to_plot)
    {
        (void) in_phase_queue.get(in_phase_memory, frame_size);
        (void) qu_phase_queue.get(qu_phase_memory, frame_size);
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

    if (!in_res || !qu_res)
    {
        if (queue_full_warnings++ % 1000 == 0)
        {
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
    }
}

const char * Plottable_CostasLoop::name()
{
    return CostasLoop::name();
}

void Plottable_CostasLoop::dispatch(RadioMsg * msg)
{
    CostasLoop::dispatch(msg);
    switch(msg->type)
    {
        case CMD_STOP:
            LOG("request_quit()\n");
            requeust_quit();
            LOG("join()\n");
            join();
            LOG("join successful...\n");
            break;

        default:
            break;
    }
}

