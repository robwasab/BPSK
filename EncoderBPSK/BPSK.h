#ifndef __BPSK_GEN_H__
#define __BPSK_GEN_H__

#include "../Module/Module.h"

class BPSK : public Module
{
public:
    BPSK(Memory * memory, 
            TransceiverCallback cb,
            void * tran,
            float fs, 
            float fc, 
            int cycles_per_bit,
            int training_bits = 0);

    Block * process(Block * block);
    const char * name();
private:
    double fs;
    double fc;
    int cycles_per_bit;
    int training_bits;
    double inc;
    double phase;
    double one_eighty;
    double samples_per_cycle;
    double phase_per_bit;
};
#endif
