#ifndef __PULSESHAPE_H__
#define __PULSESHAPE_H__

#include "../PlotController/DataSource.h"
#include "../Module/Module.h"

class Pulseshape: public Module, public DataSource
{
public:
    Pulseshape(Memory * memory, 
            Module * next, 
            float beta = 0.5, 
            float M = 101);
    ~Pulseshape();
    
    // Module
    Block * process(Block * block);
    const char * name();

    // DataSource
    size_t size() {
        return len;
    }

    AFPoint get_data(size_t index) {
        AFPoint p;
        p.x = index;
        p.y = pulseshape[index];
        return p;
    }

    void next() {
    }

    bool valid() {
        return true;
    }

    AFPoint get_origin() {
        AFPoint p;
        p.x = 0;
        p.y = min;
        return p;
    }

    AFPoint get_lengths() {
        AFPoint p;
        p.x = len;
        p.y = max - min;
        return p;
    }

private:
    float calculate();
    void slide();
    float * pulseshape;
    float * history;
    int sym_delay;
    float m;
    float min;
    float max;
    size_t len;
    void srrc(int symbol_delay, float beta, float m);
};

#endif
