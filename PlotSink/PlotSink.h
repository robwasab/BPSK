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

    Block * process(Block * b) 
    {
        float ** iter;
        size_t n;

        lock();

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
        lock();
        p.x = index;
        p.y = data[index];
        unlock();
        return p;
    }

    void next() {
        lock();
        if (!_valid) {
            _valid = true;
        }
        unlock();
    }

    bool valid() {
        bool ret;
        lock();
        ret = _valid;
        unlock();
        return ret;
    }

    Point get_origin() {
        Point p;
        lock();
        p.x = 0;
        p.y = min;
        unlock();
        return p;
    }

    Point get_lengths() {
        Point p;
        lock();
        p.x = len;
        p.y = max - min;
        unlock();
        return p;
    }

private:
    Block * block;
    float * data;
    float max;
    float min;
    size_t len;
    bool _valid;
    pthread_mutex_t mutex;
};

#endif
