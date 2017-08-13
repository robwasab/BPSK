#ifndef __PLOTTABLE_BPSK_DECODER_H__
#define __PLOTTABLE_BPSK_DECODER_H__

#include "BPSKDecoder.h"
#include "../PlotController/DataSource.h"
#include "../PlotController/PlotController.h"

#define PLOTTABLE_BPSK_DECODER_RESET_SIGNAL 0
#define PLOTTABLE_BPSK_DECODER_HIGH_PASS_SIGNAL 1

class Plottable_BPSKDecoder : public BPSKDecoder, public DataSource
{
public:
    Plottable_BPSKDecoder(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            float fs, 
            float fc, 
            bool * prefix,
            size_t prefix_len,
            int cycles_per_bit,
            float threshold,
            const uint16_t crc_table[],
            int signal_type = PLOTTABLE_BPSK_DECODER_RESET_SIGNAL);

    ~Plottable_BPSKDecoder();

    const char * name();
    /* BPSKDecoder Overridable Methods */
    void fwrdResetSignal(float signal);
    void fwrdHighPassSignal(float signal);
    void dispatch(RadioMsg * msg);

    /* Data Source Methods */
    size_t size();
    Point get_data(size_t index);
    void next();
    Point get_origin();
    Point get_lengths();
    bool valid();
    int get_updateInterval(); // return ms

private:
    int signal_type;
    size_t chunk;
    Queue<float> signal_queue;
    int update_interval;
    float * signal_memory;
};

#endif

