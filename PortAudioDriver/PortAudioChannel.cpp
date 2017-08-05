#include "PortAudioChannel.h"

PortAudioChannel::PortAudioChannel():
    source(64),
    tx_block(NULL)
{
}

void PortAudioChannel::add(Block * block)
{
    source.add(block);
}

void PortAudioChannel::process_rx_buffer(const float rx_buffer[], size_t len)
{
    return;
}

void PortAudioChannel::callback(float tx_buffer[], const float rx_buffer[], size_t len)
{
    float ** tx_iter;
    size_t start;
    size_t n;

    start = 0;
    if (source.size() > 0)
    {
        Block * block;
        source.get(&block);
        if (tx_block) 
        {
            tx_iter = tx_block->get_iterator();
            tx_buffer[0] = **tx_iter;
            start = 1;
            tx_block->free();
        }
        tx_block = block;
        tx_block->reset();
        LOG("tx_block size: %zu\n", tx_block->get_size());
    }

    if (tx_block) 
    {
        tx_iter = tx_block->get_iterator();

        for (n = start; n < len; ++n)
        {
            tx_buffer[n] = **tx_iter; 
            tx_block->next();
        }
    }
    process_rx_buffer(rx_buffer, len);
}

