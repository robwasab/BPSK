#ifndef __PLOT_SINK_H__
#define __PLOT_SINK_H__

#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "../PlotController/DataSource.h"
#include "../Memory/Block.h"
#include "../Queue/Queue.h"

const char __PLOT_SINK_NAME__[] = "dBm PlotSink";

#define PLOT_SINK_DB_MAX 30
#define PLOT_SINK_DB_MIN -30

class PlotSink : public Module, public DataSource
{
public:
    PlotSink(Memory * memory, 
            TransceiverCallback transceiver_cb, 
            void * transceiver,
            double fs):
    Module(memory, transceiver_cb, transceiver),
    queue(BUFFER_SIZE(fs))
    {
        queue_full_warnings = 0;
        frame_size = FRAME_SIZE(fs);
        update_interval = UPDATE_INTERVAL_MS;
        data = new float[frame_size];
        memset(data, 0, sizeof(float) * frame_size);
    }

    ~PlotSink() 
    {
        delete [] data;
    }

    const char * name() 
    {
        return __PLOT_SINK_NAME__;
    }

    Block * process(Block * b) 
    {
        float ** iter;

        iter = b->get_iterator();

        b->reset();

        float sample;
        bool res;
        do
        {
            sample = **iter;
            sample = fabs(sample);
            sample = sample < 0.001 ? 0.001 : sample;
            sample = 10.0 * log10f(sample) + 30.0;
            res = queue.add(sample);

            if (!res)
            {
                if (queue_full_warnings++ % 1000 == 0)
                {
                    ERROR("Signal Queue Full\n");
                }
            }

        } while(b->next());

        return b;
    }

    // DataSource
    size_t size() 
    {
        return frame_size;
    }

    AFPoint get_data(size_t index) 
    {
        AFPoint p;
        p.x = index;
        p.y = data[index];
        return p;
    }

    void next() 
    {
    }

    bool valid() 
    {
        bool enough_data = queue.size() >= frame_size; 

        if (enough_data)
        {
            queue.get(data, frame_size);
            return false;
        }
        return true;
    }

    AFPoint get_origin() {
        AFPoint p;
        p.x = 0;
        p.y = PLOT_SINK_DB_MIN;
        return p;
    }

    AFPoint get_lengths() {
        AFPoint p;
        p.x = frame_size;
        p.y = PLOT_SINK_DB_MAX - PLOT_SINK_DB_MIN;
        return p;
    }

    void dispatch(RadioMsg * msg)
    {
        Module::dispatch(msg);
        switch(msg->type)
        {
            case CMD_STOP:
                LOG("request_quit()\n");
                requeust_quit();
                LOG("join()\n");
                //join();
                LOG("join successful...\n");
                break;

            default:
                break;
        }
    }


private:
    Queue<float> queue;
    int queue_full_warnings;
    size_t frame_size;
    int update_interval;
    float * data;
};

#endif
