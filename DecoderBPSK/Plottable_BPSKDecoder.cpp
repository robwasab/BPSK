#include "Plottable_BPSKDecoder.h"
#include <math.h>
#include <string.h>

Plottable_BPSKDecoder::Plottable_BPSKDecoder(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            float fs, 
            float fc, 
            bool * prefix,
            size_t prefix_len,
            int cycles_per_bit,
            float threshold,
            const uint16_t crc_table[],
            int signal_type):
    BPSKDecoder(memory, cb, trans, fs, fc, prefix, prefix_len, cycles_per_bit, threshold, crc_table),
    signal_type(signal_type),
    chunk(1024),
    signal_queue(1 << 18)
{
    update_interval = (int) round(chunk/fs);
    signal_memory = new float[chunk];
    memset(signal_memory, 0, sizeof(float) * chunk);
}

void Plottable_BPSKDecoder::fwrdResetSignal(float signal)
{
    switch (signal_type)
    {
        case PLOTTABLE_BPSK_DECODER_RESET_SIGNAL:
            signal_queue.add(signal);
            break;
        default:
            break;
    }
}

void Plottable_BPSKDecoder::fwrdHighPassSignal(float signal)
{
    switch (signal_type)
    {
        case PLOTTABLE_BPSK_DECODER_HIGH_PASS_SIGNAL:
            signal_queue.add(signal);
            break;
        default:
            break;
    }
}

int Plottable_BPSKDecoder::get_updateInterval()
{
    return update_interval;
}

Plottable_BPSKDecoder::~Plottable_BPSKDecoder()
{
    delete [] signal_memory;
}

size_t Plottable_BPSKDecoder::size()
{
    return chunk;
}

Point Plottable_BPSKDecoder::get_data(size_t index)
{
    Point p;
    p.x = index;
    p.y = signal_memory[index];
    return p;
}

void Plottable_BPSKDecoder::next()
{

}

Point Plottable_BPSKDecoder::get_origin()
{
    Point p;
    p.x = 0; 
    p.y = -1.5;
    return p;
}

Point Plottable_BPSKDecoder::get_lengths()
{
    Point p;
    p.x = chunk;
    p.y = 3.0;
    return p;
}

bool Plottable_BPSKDecoder::valid()
{
    bool valid = !(signal_queue.size() >= chunk);

    if (!valid)
    {
        (void) signal_queue.get(signal_memory, chunk);
        return false;
    }
    return true;
}

const char * Plottable_BPSKDecoder::name()
{
    return BPSKDecoder::name();
}
