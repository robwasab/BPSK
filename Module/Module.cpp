#include "Module.h"

Module::Module(Memory * memory, TransceiverCallback cb, void * transceiver) :
    memory(memory),
    transceiver_cb(cb),
    transceiver(transceiver)
{
}

Module::~Module() 
{
}

void Module::handoff(Block * block, uint8_t thread_id)
{
    RadioData msg;
    msg.set_id(id);
    msg.set_thread_id(thread_id);
    msg.set_block(block);
    transceiver_cb(transceiver, &msg);
}

void Module::broadcast(RadioMsg * msg)
{
    transceiver_cb(transceiver, msg);
}

void Module::dispatch(RadioMsg * msg)
{
    RadioData * data;
    Block * block;

    data = (RadioData *) msg;


    switch(msg->type)
    {
        case PROCESS_DATA:
            block = data->get_block();

            block = process(block);

            if (block != NULL)
            {
                handoff(block, data->get_tid());
            }
            break;

        case CMD_START:
        case CMD_STOP:
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
            LOG("%s\n", RadioMsgString[msg->type]);
            break;
        default:
            break;
    }
}

