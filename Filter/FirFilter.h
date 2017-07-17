#ifndef __FIR_FILTER_H__
#define __FIR_FILTER_H__

#include "../Module/Module.h"

class FirFilter : public Module
{
public:
    /* simply saves a references to coefs */
    FirFilter(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver,
            size_t n, 
            const double coefs[]);
    ~FirFilter();
    Block * process(Block * block);
    const char * name();
    float work(float input);
    void reset();
protected:
    const double * coefs;
    size_t n;
private:
    float * save;
};
#endif
