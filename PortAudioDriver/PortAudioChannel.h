#ifndef __PORT_AUDIO_CHANNEL_H__
#define __PORT_AUDIO_CHANNEL_H__

#include "../Module/Module.h"
#include "../Queue/Queue.h"

class PortAudioChannel
{
public:
    PortAudioChannel();

    /* Add a transmit block */
    void add(Block * block);

    /* Called in Port Audio Driver */
    void callback(float tx_buffer[], const float rx_buffer[], size_t len);

    /* Needs to implement */
    virtual void process_rx_buffer(const float rx_buffer[], size_t len);

    void load();

private:
    Queue<Block *> source;
    Block * tx_block;
    bool load_block;
};

#endif
