#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "../RadioMsg/RadioMsg.h"
#include "../Module/Module.h"
#include "../switches.h"

#ifdef SIMULATE
#include <random>
#include "pthread.h"
#endif

#include "../PortAudioDriver/PortAudioChannel.h"

class Channel : public Module, public PortAudioChannel
{
public:
    Channel(Memory * rx_memory, TransceiverCallback cb, void * transceiver);
    void dispatch(RadioMsg * msg);
    const char * name();
    void set_noise_level(double noise_level_db);
#ifdef SIMULATE 
    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
    pthread_mutex_t mutex;
#endif
    /* Implement PortAudioChannel receive callback */
    void process_rx_buffer(const float rx_buffer[], size_t len);

private:
    Block * process(Block * block);
    int handle;
};

#endif
