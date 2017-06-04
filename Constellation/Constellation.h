#ifndef __CONSTELLATION_H__
#define __CONSTELLATION_H__

#include "../Module/Module.h"
#include "../PlotController/DataSource.h"

class Constellation : public DataSource, public Module
{
public:
    Constellation(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            double fs,
            size_t chunk);

    ~Constellation();
    const char * name();
    size_t size();
    Point get_data(size_t index);
    void next();
    Point get_origin();
    Point get_lengths();
    bool valid();
    int get_updateInterval();

private:
    double fs;
    size_t chunk;
    int update_interval;
    Queue<float> in_phase_queue;
    Queue<float> qu_phase_queue;
    float * in_phase_memory;
    float * qu_phase_memory;

    Block * process(Block * block);
};

#endif
