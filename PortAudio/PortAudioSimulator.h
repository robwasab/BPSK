#ifndef __PORT_AUDIO_TRANSMIT_BLOCK_H__
#define __PORT_AUDIO_TRANSMIT_BLOCK_H__

#include <pthread.h>
#include "TransmitBlock.h"
#include "../Queue/Queue.h"
#include "../Transmitter/Prefix.h"
#include "../Transmitter/BPSK.h"
#include "../Memory/Memory.h"
#include "../TaskScheduler/TaskScheduler.h"

class PortAudioSimulator 
{
public:
    PortAudioSimulator( TaskScheduler * rx_scheduler,
            Memory * rx_memory,
            Module ** tx_modules,
            Module * rx_module);

    ~PortAudioSimulator();
    void start();
    void stop();
    void add(Block * ascii);

    TaskScheduler * rx_scheduler;
    Memory * rx_memory;
    Module * rx_module;
    Queue<Block *> source;
    Module ** tx_modules;
    size_t tx_modules_size;
    bool quit;
    pthread_mutex_t mutex;
private:
    pthread_t thread;
};
#endif
