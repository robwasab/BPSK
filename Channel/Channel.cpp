#include <math.h>
#include "Channel.h"
#include "../PortAudioDriver/PortAudioDriver.h"

#ifdef SIMULATE
#include <pthread.h>
#endif

 
void Channel::process_rx_buffer(const float rx_buffer[], size_t len)
{
    Block * rx_block;
    float ** rx_iter;
    size_t n;
    RadioData msg;

    rx_block = memory->allocate(len);
    rx_iter  = rx_block->get_iterator();

    #ifdef SIMULATE
    pthread_mutex_lock(&mutex);
    #endif

    for (n = 0; n < len; n++)
    {
        **rx_iter = rx_buffer[n];
        #ifdef SIMULATE
        **rx_iter += distribution(generator);
        #endif
        rx_block->next();
    }

    #ifdef SIMULATE
    pthread_mutex_unlock(&mutex);
    #endif

    rx_block->reset();

    handoff(rx_block, 0);
}


double db_noise(double db)
{
    double std_dev = sqrt(pow(10.0, db/10.0) / 2.0);
    LOG("Standard Deviation: %.3f\n", std_dev);
    return std_dev;
}

void Channel::set_noise_level(double noise_level_db)
{
    #ifdef SIMULATE
    std::normal_distribution<double> new_distribution(0.0, db_noise(noise_level_db));
    pthread_mutex_lock(&mutex);
    distribution = new_distribution;
    pthread_mutex_unlock(&mutex);
    #endif
}

Channel::Channel(Memory * memory, TransceiverCallback cb, void * transceiver):
    Module(memory, cb, transceiver),
    PortAudioChannel()
    #ifdef SIMULATE
    ,distribution(0.0, db_noise(-20.0))
    #endif
{
    handle = PortAudio_init(this);

    #ifdef SIMULATE
    pthread_mutex_init(&mutex, NULL);
    #endif
}

Block * Channel::process(Block * block)
{
    return block;
}

void Channel::dispatch(RadioMsg * msg)
{
    double noise_level_db;
    RadioData * data;
    
    data = (RadioData *) msg;
    noise_level_db = 0.0;

    switch(msg->type)
    {
        case PROCESS_DATA:
            add(data->get_block());
            break;

        case CMD_START:
            LOG("starting %s...\n", name());
            PortAudio_start();
            break;

        case CMD_STOP:
            PortAudio_stop(handle);
            break;

        case CMD_SET_NOISE_LEVEL:
            #ifdef SIMULATE
            memcpy(&noise_level_db, msg->args, sizeof(double));
            LOG("Setting noise level to %.3lf dB\n", noise_level_db);
            set_noise_level(noise_level_db);
            #endif
            break;

        case NOTIFY_MSG_EXHAUSTED:
            /* indicate that it is ok to load new message */
            load();
            break;

        default:
            break;
    }
}

static char __name__[] = "Channel";

const char * Channel::name()
{
    return __name__;
}

