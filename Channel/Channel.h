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
    
    /* Functions used by the audio hardware interface */
    /* Add a transmit block */
    void add(Block * block);
    
    /* Called in Audio Driver */
    void transmit_callback(float tx_buffer[], size_t len);
    void receive_callback(const float rx_buffer[], size_t len);
        
    void load();


private:
    Block * process(Block * block);
    int handle;
    
    /* Variables used by the audio hardware interface */
    Queue<Block *> source;
    Block * tx_block;
    bool load_block;
};

#endif
