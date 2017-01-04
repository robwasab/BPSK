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
    PlotSink(Module * next) :
    Module(NULL, next)
    {
        block = NULL;
        data = NULL;
        max = +1.0;
        min = -1.0;
        len = 0;
        _valid = true;
        processing = false;
        pthread_mutex_init(&mutex, NULL);
    }

    ~PlotSink() {
        if (data) {
            delete [] data;
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

        if (data) {
            delete [] data;
        }

        data = new float[len];

        b->reset();

        b->print();

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
        p.y = max - min;
        return p;
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
};

#endif
