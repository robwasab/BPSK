#include "Channel.h"
#include "../PortAudioDriver/PortAudioDriver.h"

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

    for (n = 0; n < frames; n++)
    {
        **rx_iter = rx_buffer[n];
        rx_block->next();
    }

    rx_block->reset();

    self->handoff(rx_block, 0);
}

Channel::Channel(Memory * memory, TransceiverCallback cb, void * transceiver):
    Module(memory, cb, transceiver)
{
    PortAudio_init(this, on_receive);
}

Block * Channel::process(Block * block)
{
    return block;
}

void Channel::dispatch(RadioMsg * msg)
{
    RadioData * data = (RadioData *) msg;
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

        case CMD_RESET_ALL:
        case CMD_RESET_TRANSMITTER:
        case CMD_RESET_RECEIVER:
        case CMD_SET_TRANSMIT_CHANNEL:
        case CMD_SET_RECEIVE_CHANNEL:
        case NOTIFY_PLL_LOST_LOCK:
        case NOTIFY_PACKET_HEADER_DETECTED:
        case NOTIFY_RECEIVER_RESET_CONDITION_DETECTED:
        case NOTIFY_DATA_RECEIVED:
        case NOTIFY_USER_REQUEST_QUIT:
            break;
    }
}

static char __name__[] = "Channel";

const char * Channel::name()
{
    return __name__;
}

