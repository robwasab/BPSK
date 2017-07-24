#ifndef __BPSK_DECODER_H__
#define __BPSK_DECODER_H__

#include "../Module/Module.h"
#include "../CostasLoop/CostasLoop.h"
#include "../PlotController/DataSource.h"
#include "HighPass.h"

enum DecoderState 
{
    ACQUIRE, 
    LOOK_FOR_HEADER, 
    READ_SIZE, 
    COLLECT_BITS,
};

class BPSKDecoder : public Module
{
public:
    BPSKDecoder(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            float fs, 
            float fc, 
            bool * prefix,
            size_t prefix_len,
            int cycles_per_bit,
            float threshold = 0.75);

    /* Module Methods */
    const char * name();
    Block * process(Block * block);
    void dispatch(RadioMsg * msg);

    /* BPSKDecoder Overridable Method */
    virtual void plot_debug_signal(float signal);

private:
    /* BPSK Decoder Vars */
    uint32_t prefix;            /* Maximal length prefix */
    uint32_t prefix_mask;
    int sample_period;          /* Samples per bit */
    float fs;                   /* 44.1 kHz */
    uint64_t trail_samples;     /* Shift register to hold onto the last 32 bits */
    int trail_samples_len;
    float threshold;            /* Threshold to trigger the receiver */
    DecoderState state;

    Block * msg;
    RC_LowPass timer;

    HighPass filter;
    uint32_t shift_register;
    int count;
    bool last_bit;

    uint8_t k;
    uint8_t byte;
    float ** msg_iter;
    uint8_t byte_msg[256];

    /* Private Methods */
    bool majority_vote();
    void add_level(bool level);
    void print_shift_register(uint32_t shift_register);
};

#endif
