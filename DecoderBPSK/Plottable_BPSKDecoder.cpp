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
    frame_size(FRAME_SIZE(fs)),
    signal_queue(BUFFER_SIZE(fs))
{
    queue_full_warnings = 0;
    update_interval = UPDATE_INTERVAL_MS;
    signal_memory = new float[frame_size];
    memset(signal_memory, 0, sizeof(float) * frame_size);
}

void Plottable_BPSKDecoder::addSignal(float signal)
{
    bool res = signal_queue.add(signal);

    if (!res)
    {
        if (queue_full_warnings++ % 1000 == 0)
        {
            ERROR("Signal Queue Full!\n");
        }
    }
}

void Plottable_BPSKDecoder::fwrdResetSignal(float signal)
{
    switch (signal_type)
    {
        case PLOTTABLE_BPSK_DECODER_RESET_SIGNAL:
            addSignal(signal);
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
            addSignal(signal);
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
    return frame_size;
}

AFPoint Plottable_BPSKDecoder::get_data(size_t index)
{
    AFPoint p;
    p.x = index;
    p.y = signal_memory[index];
    return p;
}

void Plottable_BPSKDecoder::next()
{

}

AFPoint Plottable_BPSKDecoder::get_origin()
{
    AFPoint p;
    p.x = 0; 
    p.y = -1.5;
    return p;
}

AFPoint Plottable_BPSKDecoder::get_lengths()
{
    AFPoint p;
    p.x = frame_size;
    p.y = 3.0;
    return p;
}

bool Plottable_BPSKDecoder::valid()
{
    bool readFromQueue = signal_queue.size() >= frame_size;

    if (readFromQueue)
    {
        (void) signal_queue.get(signal_memory, frame_size);
        return false;
    }
    return true;
}

const char * Plottable_BPSKDecoder::name()
{
    return BPSKDecoder::name();
}

void Plottable_BPSKDecoder::dispatch(RadioMsg * msg)
{
    BPSKDecoder::dispatch(msg);

    switch(msg->type)
    {
        case CMD_STOP:
            LOG("request_quit()\n");
            requeust_quit();
            LOG("join()\n");
            join();
            LOG("join successful...\n");
            break;

        default:
            break;
    }
}

