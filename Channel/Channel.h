#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "../RadioMsg/RadioMsg.h"
#include "../Module/Module.h"
#include "../switches.h"

#ifdef SIMULATE
#include <random>
#include "pthread.h"
#endif


class Channel : public Module
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
private:
    Block * process(Block * block);
};

#endif
