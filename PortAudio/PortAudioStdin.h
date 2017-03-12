#ifndef __PORT_AUDIO_STDIN_H__
#define __PORT_AUDIO_STDIN_H__

#include <pthread.h>
#include "../Memory/Memory.h"
#include "../Memory/Block.h"
#include "PortAudioSimulator.h"
#include "../switches.h"

class PortAudioStdin
{
public:
    PortAudioStdin(Memory * memory, PortAudioSimulator * portaudio);
    void start(bool block = true);
    Block * process(Block * ascii);

    Memory * memory;
    PortAudioSimulator * portaudio;
private:
    pthread_t main;
};

#endif
