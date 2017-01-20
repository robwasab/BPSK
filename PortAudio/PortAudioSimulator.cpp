#include "PortAudioSimulator.h"
#include "../Memory/Block.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void * PortAudioSimulator_loop(void * arg);

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
    PortAudioSimulator * self = (PortAudioSimulator *) arg;
    unsigned long sleep_time = (unsigned long) round(size/44.1E3*1E6);
    //sleep_time = 1E5;

    //int count = 0;
    Block * tx_block = NULL;
    float * tx_buffer = (float *) malloc(sizeof(float) * size);
    float * rx_buffer = tx_buffer;

    while(1)
    {
        usleep(sleep_time);
        //printf("[%d] transmitting data...\n", count++);
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

            for (size_t n = 0; n < size; ++n)
            {
                tx_buffer[n] = **tx_iter;
                tx_block->next();
            }

            Block * rx_block = self->rx_memory->allocate(size);
            float ** rx_iter = rx_block->get_iterator();

            for (size_t n = 0; n < size; ++n)
            {
                **rx_iter = rx_buffer[n];
                rx_block->next();
            }

            rx_block->reset();

            self->rx_scheduler->add_module(self->rx_module, rx_block);

            /*
            for (size_t n = 0; n < 10; ++n)
            {
                printf("[%zu] %.3f\n", n, tx_buffer[n]);
            }

            printf("...\n...\n...\n");

            for (size_t n = size-10; n < size; ++n)
            {
                printf("[%zu] %.3f\n", n, tx_buffer[n]);
            }
            */
        }

        pthread_mutex_lock(&self->mutex);
        if (self->quit) 
        {
            pthread_mutex_unlock(&self->mutex);
            break;
        }
        pthread_mutex_unlock(&self->mutex);
    }
    printf("PortAudio quitting...\n");
    return NULL;
}

static
int PortAudio_callback(const void * input, 
    void * output, 
    unsigned long frames, 
    const PaStreamCallbackTimeInfo * timeInfo, 
    PaStreamCallbackFlags statusFlags, 
    void * arg)
{

}
