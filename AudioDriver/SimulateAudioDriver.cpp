//
//  SimulateAudioDriver.cpp
//  AudioFrequency
//
//  Created by Robby Tong on 10/22/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "../Module/Module.h"
#include "../Queue/Queue.h"
#include "../switches.h"
#include "../Channel/Channel.h"

#include "AudioDriver.h"

#ifdef SIMULATE

#define MAX_CHANNELS 10

static int occupied_channels = 0;

static Channel * channels[MAX_CHANNELS] = {NULL};

static pthread_mutex_t mutex;
static bool quit = false;

static pthread_t thread;

typedef enum
{
    paContinue = 0x00,
    paAbort,
    paComplete,
} PaReturnType;


static void * PortAudioSimulator_loop(void * arg);
static PaReturnType PortAudio_callback(const void *input,
                                       void *output,
                                       unsigned long frames,
                                       void * arg );


#define LOCK pthread_mutex_lock(&mutex);
#define UNLOCK pthread_mutex_unlock(&mutex);

/*
 * Initialize Port Audio with a channel.
 * returns the handle number.
 * returns -1 if no more available channles.
 */
int AudioDriver_init(Channel * channel)
{
    if (occupied_channels < 1)
    {
        pthread_mutex_init(&mutex, NULL);
        occupied_channels = 1;
    }
    else if (occupied_channels >= MAX_CHANNELS)
    {
        return -1;
    }
    else
    {
        occupied_channels++;
    }
    channels[occupied_channels - 1] = channel;
    
    LOG("Port Audio handle: %d\n", occupied_channels-1);
    
    return occupied_channels - 1;
}


void AudioDriver_start()
{
    static bool started = false;
    
    if (started)
    {
        LOG("Port Audio already started\n");
        return;
    }
    else
    {
        LOG("Starting Port Audio!\n");
        started = true;
    }
    pthread_create(&thread, NULL, PortAudioSimulator_loop, NULL);
    return;
}

void AudioDriver_stop(int handle)
{
    if (handle >= occupied_channels || handle < 0)
    {
        LOG("Invalid Handle %d...\n", handle);
        return;
    }
    
    pthread_mutex_lock(&mutex);
    
    if (channels[handle] != NULL)
    {
        channels[handle] = NULL;
        occupied_channels--;
    }
    
    pthread_mutex_unlock(&mutex);
    
    if (occupied_channels > 0)
    {
        return;
    }
    else
    {
        LOG("Shutting down Port Audio!\n");
    }
    
    pthread_mutex_lock(&mutex);
    quit = true;
    pthread_mutex_unlock(&mutex);
    
    pthread_join(thread, NULL);
}

void * PortAudioSimulator_loop(void * arg)
{
    size_t size = 1024;
    unsigned int sleep_time = (unsigned int) round(size/44.1E3*1E6);
    
    float * tx_buffer = (float *) malloc(sizeof(float) * size);
    float * rx_buffer = tx_buffer;
    unsigned long frames = size;
    
    bool quit = false;
    PaReturnType result;
    
    while(!quit)
    {
        usleep(sleep_time);
        result = PortAudio_callback(rx_buffer, tx_buffer, frames, arg);
        
        switch (result)
        {
            case paContinue:
                continue;
                
            case paAbort:
                ERROR("PortAudio returned Error!\n");
                break;
                
            case paComplete:
                LOG("PortAudio completed!\n");
                quit = true;
                break;
        }
    }
    LOG("PortAudio quitting...\n");
    return NULL;
}

static
PaReturnType PortAudio_callback(const void *input,
                       void *output,
                       unsigned long frames,
                       void * arg )
{
    /* assume mono playback */
    float * tx_buffer = (float *) output;
    const float * rx_buffer = (const float *) input;
    size_t k;
    
    memset(tx_buffer, 0, sizeof(float) * frames);
    
    double scale = 0.0;
    
    for (k = 0; k < MAX_CHANNELS; k++)
    {
        if (channels[k] != NULL)
        {
            channels[k]->transmit_callback(tx_buffer, frames);
            channels[k]->receive_callback(rx_buffer, frames);
            scale += 1.0;
        }
    }
    
    for (k = 0; k < frames; k++)
    {
        tx_buffer[k] /= scale;
    }
    
    pthread_mutex_lock(&mutex);
    if (quit)
    {
        pthread_mutex_unlock(&mutex);
        return paComplete;
    }
    pthread_mutex_unlock(&mutex);
    
    #ifdef PRINT_CPU_LOAD
    double load;
    if (stream)
    {
        load = Pa_GetStreamCpuLoad(stream);
        LOG("CPU load: %.3lf\r", 100.0 * load);
    }
    #endif
    
    return paContinue;
}

#endif
