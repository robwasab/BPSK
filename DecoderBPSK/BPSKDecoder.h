#ifndef __BPSK_DECODER_H__
#define __BPSK_DECODER_H__

#include "../Module/Module.h"
#include "../CostasLoop/CostasLoop.h"
#include "../PlotController/DataSource.h"

class BPSKDecoder : public Module
{
public:
    BPSKDecoder(Memory * memory, 
            Module * next, 
            float fs, 
            float fc, 
            bool * prefix,
            size_t prefix_len,
            int cycles_per_bit,
            bool first_bit,
            float threshold = 0.5);

    const char * name();
    Block * process(Block * block);
    DataSource * get_source(SignalType type);
private:
    bool first_bit;
    uint32_t prefix;
    uint32_t prefix_mask;
    int sample_period;
    float fs;
    uint64_t trail_samples;
    int trail_samples_len;
    float threshold;
    bool majority_vote();
    void add_level(bool level);
    void print_shift_register(uint32_t shift_register);
};

#endif
