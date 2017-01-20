#include "PortAudioSimulator.h"
#include "../Memory/Block.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <portaudio.h>

void * PortAudioSimulator_loop(void * arg);
int PortAudio_callback(
    const void *input, 
    void *output,
    unsigned long frames,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void * arg );

size_t calc_size(Module ** modules)
{
    size_t count = 0;
    while (modules[count] != NULL) {
        count += 1;
    }
    LOG("count: %zu\n", count);
    return count;
}

PortAudioSimulator::PortAudioSimulator(
        TaskScheduler * rx_scheduler,
        Memory *  rx_memory,
        Module ** tx_modules,
        Module * rx_module):
    rx_scheduler(rx_scheduler),
    rx_memory(rx_memory),
    rx_module(rx_module),
    source(64)
{
    LOG("Calculating size...\n");
    tx_modules_size = calc_size(tx_modules);

    LOG("Copying tx_modules\n");
    this->tx_modules = new Module*[tx_modules_size];

    memcpy(this->tx_modules, tx_modules, sizeof(Module *) * this->tx_modules_size);

    pthread_mutex_init(&mutex, NULL);
}

void PortAudioSimulator::start()
{
    pthread_create(&thread, NULL, PortAudioSimulator_loop, this);
}

PortAudioSimulator::~PortAudioSimulator()
{
    stop();
    delete [] tx_modules;
}

void PortAudioSimulator::stop()
{
    pthread_mutex_lock(&mutex);
    quit = true;
    pthread_mutex_unlock(&mutex);
    pthread_join(thread, NULL);
}

void PortAudioSimulator::add(Block * block)
{
    for (size_t k = 0; k < tx_modules_size; ++k)
    {
        block = tx_modules[k]->process(block);
    }
    source.add(block);
}

void * PortAudioSimulator_loop(void * arg)
{
    size_t size = 1024;
    unsigned long sleep_time = (unsigned long) round(size/44.1E3*1E6);
    //sleep_time = 1E5;

    //int count = 0;
    float * tx_buffer = (float *) malloc(sizeof(float) * size);
    float * rx_buffer = tx_buffer;
    unsigned long frames = size;

    PaStreamCallbackTimeInfo time_info;
    PaStreamCallbackFlags status_flags = 0;
    bool quit = false;
    int result;

    while(!quit)
    {
        time_info.inputBufferAdcTime = 0.0;
        time_info.currentTime = 0.0;
        time_info.outputBufferDacTime = 0.0;
        usleep(sleep_time);
        result = PortAudio_callback(
                rx_buffer, tx_buffer, frames, &time_info, status_flags, arg);

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

int PortAudio_callback(
    const void *input, 
    void *output,
    unsigned long frames,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void * arg )
{
    /* assume mono playback */
    static Block * tx_block = NULL;
    float * tx_buffer = (float *) output;
    const float * rx_buffer = (const float *) input;

    PortAudioSimulator * self = (PortAudioSimulator *) arg;

    /* Parse Error Messages TODO */
    /*
    PaTime input_time = timeInfo->inputBufferAdcTime;
    PaTime invok_time = timeInfo->currentTime;
    PaTime outpu_time = timeInfo->outputBufferDacTime;

    printf("input  time: %lf\n", input_time);
    printf("invoke time: %lf\n", invok_time);
    printf("output time: %lf\n", outpu_time);
    */

    if (statusFlags & paInputUnderflow) {
        LOG("Input Underflow, not enough input data.\n");
    }
    else if (statusFlags & paInputOverflow) {
        LOG("Input Overflow, callback taking too much time.\n");
    }
    else if (statusFlags & paOutputUnderflowed) {
        LOG("Output Underflow, callback taking too much time.\n");
    }
    else if (statusFlags & paOutputOverflow) {
        LOG("Output Overflow, too much output data.\n");
    }
    else if (statusFlags & paPrimingOutput) {
        LOG("Priming the stream.\n");
    }


    if (self->source.size() > 0)
    {
        Block * block;
        self->source.get(&block);
        if (tx_block) {
            tx_block->free();
        }
        tx_block = block;
        tx_block->reset();
        LOG("tx_block size: %zu\n", tx_block->get_size());
    }

    if (tx_block) 
    {
        float ** tx_iter = tx_block->get_iterator();

        for (size_t n = 0; n < frames; ++n)
        {
            tx_buffer[n] = **tx_iter;
            tx_block->next();
        }

        Block * rx_block = self->rx_memory->allocate(frames);
        float ** rx_iter = rx_block->get_iterator();

        for (size_t n = 0; n < frames; ++n)
        {
            **rx_iter = rx_buffer[n];
            rx_block->next();
        }

        rx_block->reset();

        self->rx_scheduler->add_module(self->rx_module, rx_block);
    }
    else 
    {
        memset(tx_buffer, 0, sizeof(float) * frames);
    }

    pthread_mutex_lock(&self->mutex);
    if (self->quit) 
    {
        pthread_mutex_unlock(&self->mutex);
        return paComplete;
    }
    pthread_mutex_unlock(&self->mutex);

    return paContinue;
}
