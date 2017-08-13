#ifndef __PLOT_SINK_H__
#define __PLOT_SINK_H__

#include <stdint.h>
#include <pthread.h>
#include <math.h>
#include "../PlotController/DataSource.h"
#include "../Memory/Block.h"

const char __PLOT_SINK_NAME__[] = "PlotSink";

class PlotSink : public Module, public DataSource
{
public:
    PlotSink(Memory * memory, 
            TransceiverCallback transceiver_cb, 
            void * transceiver):
    Module(memory, transceiver_cb, transceiver)
    {
        block = NULL;
        data = NULL;
        max = +1.0;
        min = -1.0;
        len = 0;
        _valid = true;
        processing = false;
        pthread_mutex_init(&mutex, NULL);
        data_alloc = false;
    }

    ~PlotSink() {
        if (data_alloc) {
            delete [] data;
            data_alloc = false;
        }
    }

    const char * name() {
        return __PLOT_SINK_NAME__;
    }

    void lock() {
        pthread_mutex_lock(&mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex);
    }

    bool is_processing() {
        bool ret;
        lock();
        ret = processing;
        unlock();
        return ret;
    }

    Block * process(Block * b) 
    {
        float ** iter;
        size_t n;

        lock();
        processing = true;
        unlock();

        n = 0;
        iter = b->get_iterator();
        len = b->get_size();

        if (data_alloc) {
            delete [] data;
            data_alloc = false;
        }

        data = new float[len];
        data_alloc = true;

        b->reset();

        //b->print();

        min = 100;
        max = -100;

        do
        {
            data[n] = **iter;
            if (data[n] > max) {
                max = data[n];
            }
            if (data[n] < min) {
                min = data[n];
            }
            n += 1;
        } while(b->next());

        _valid = false;

        lock();
        processing = false;
        unlock();

        return b;
    }

    // DataSource
    size_t size() {
        size_t ret;
        lock();
        ret = len;
        unlock();
        return ret;
    }

    Point get_data(size_t index) {
        Point p;
        while(is_processing());
        p.x = index;
        p.y = data[index];
        return p;
    }

    void next() {
        while(is_processing());
        if (!_valid) {
            _valid = true;
        }
    }

    bool valid() {
        bool ret;
        while(is_processing());
        ret = _valid;
        return ret;
    }

    Point get_origin() {
        Point p;
        while(is_processing());
        p.x = 0;
        p.y = min;
        return p;
    }

    Point get_lengths() {
        Point p;
        while(is_processing());
        p.x = len;
        float width;
        if (fabs(max) < fabs(min)) {
            width = 2.2 * fabs(min);
        }
        else {
            width = 2.2 * fabs(max);
        }
        p.y = width;
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
                join();
                LOG("join successful...\n");
                break;

            default:
                break;
        }
    }


private:
    bool processing;
    Block * block;
    float * data;
    float max;
    float min;
    size_t len;
    bool _valid;
    pthread_mutex_t mutex;
    bool data_alloc;
};

#endif
