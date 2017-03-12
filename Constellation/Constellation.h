#ifndef __CONSTELLATION_H__
#define __CONSTELLATION_H__

#include "../Module/Module.h"
#include "../PlotController/DataSource.h"

class Constellation : public DataSource, public Module
{
public:
    Constellation(Memory * memory, 
            Module * next, 
            double fs,
            size_t chunk);

    ~Constellation();
    Block * process(Block * sig);
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
};

#endif
