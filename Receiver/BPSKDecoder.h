#ifndef __BPSK_DECODER_H__
#define __BPSK_DECODER_H__

#include "../Module/Module.h"

class BPSKDecoder : public Module
{
public:
    BPSKDecoder(Memory * memory, 
            Module * next, 
            float fs, 
            float fc, 
            int cycles_per_bit,
            bool first_bit);

    const char * name();
    Block * process(Block * block);
private:
    int sample_period;
};

#endif
