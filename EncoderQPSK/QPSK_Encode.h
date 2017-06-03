#ifndef __QPSK_ENCODE_H__
#define __QPSK_ENCODE_H__

#include "../Module/Module.h"

class QPSK_Encode : public Module
{
public:
    QPSK_Encode(Memory * memory, 
            Module * next, 
            float fs, 
            float fc, 
            int cycles_per_sym,
            int training_cycles = 0);

    Block * process(Block * block);
    const char * name();
    ~QPSK_Encode() { }
private:
    double fs;
    double fc;
    int cycles_per_sym;
    int training_cycles;
    double inc;
    double phase;
    int    qpsk_sym;
    double samples_per_cycle;
    double phase_per_sym;
};
#endif
