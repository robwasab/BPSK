#include <math.h>
#include "Channel.h"
#include "../PortAudioDriver/PortAudioDriver.h"

#ifdef SIMULATE
#include <pthread.h>
#endif

static void on_receive(void * arg, const float rx_buffer[], unsigned long frames)
{
    Channel * self;
    Block * rx_block;
    float ** rx_iter;
    unsigned long n;
    RadioData msg;

    self = (Channel *) arg;


    rx_block = self->memory->allocate(frames);
    rx_iter  = rx_block->get_iterator();

#ifdef SIMULATE
    pthread_mutex_lock(&self->mutex);
#endif

    for (n = 0; n < frames; n++)
    {
        **rx_iter = rx_buffer[n];
#ifdef SIMULATE
        **rx_iter += self->distribution(self->generator);
#endif
        rx_block->next();
    }

#ifdef SIMULATE
    pthread_mutex_unlock(&self->mutex);
#endif

    rx_block->reset();

    self->handoff(rx_block, 0);
}


double db_noise(double db)
{
    double std_dev = sqrt(pow(10.0, db/10.0) / 2.0);
    LOG("Standard Deviation: %.3f\n", std_dev);
    return std_dev;
}

void Channel::set_noise_level(double noise_level_db)
{
    std::normal_distribution<double> new_distribution(0.0, db_noise(noise_level_db));
    pthread_mutex_lock(&mutex);
    distribution = new_distribution;
    pthread_mutex_unlock(&mutex);
}

Channel::Channel(Memory * memory, TransceiverCallback cb, void * transceiver):
    Module(memory, cb, transceiver)
#ifdef SIMULATE
    ,distribution(0.0, db_noise(-20.0))
#endif
{
    PortAudio_init(this, on_receive);

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
            PortAudio_add(data->get_block());
            break;

        case CMD_START:
            PortAudio_start();
            break;

        case CMD_STOP:
            PortAudio_stop();
            break;

        case CMD_SET_NOISE_LEVEL:
            memcpy(&noise_level_db, msg->args, sizeof(double));
            LOG("Setting noise level to %.3lf dB\n", noise_level_db);
            set_noise_level(noise_level_db);
            break;

        case CMD_RESET_ALL:
        case CMD_RESET_TRANSMITTER:
        case CMD_RESET_RECEIVER:
        case CMD_SET_TRANSMIT_CHANNEL:
        case CMD_SET_RECEIVE_CHANNEL:
        case NOTIFY_PLL_RESET:
        case NOTIFY_PACKET_HEADER_DETECTED:
        case NOTIFY_RECEIVER_RESET_CONDITION_DETECTED:
        case NOTIFY_DATA_RECEIVED:
        case NOTIFY_USER_REQUEST_QUIT:
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

