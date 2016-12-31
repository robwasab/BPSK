#ifndef __BANDPASS_H__
#define __BANDPASS_H__

#include "../Module/Module.h"

class SOSBandPass 
{
public:
    SOSBandPass(float a, float b, float c, float d);
    float work(float sig);
    void reset();
private:
    float a[3];
    float b[3];
    float y[3];
    float x[3];
};

class BandPass : public Module
{
public:
    BandPass(Memory * memory, 
            Module * next, 
            float fs,
            float fc,
            float bw,
            int order);
    ~BandPass();

    Block * process(Block * block);
    const char * name();
    float work(float sig);
    void reset();

private:
    int order;
    float bw;
    float fc;
    float fs;
    SOSBandPass ** filters;
};

#endif
