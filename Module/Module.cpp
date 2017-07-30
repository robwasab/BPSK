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
            LOG("starting %s...\n", name());
            break;

        case CMD_STOP:
            LOG("stopping %s...\n", name());
            break;

        default:
            break;
    }
}

