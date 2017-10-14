#include "ByteInputInterface.h"
#include "../CRC-16/crc.h"

ByteInputInterface::ByteInputInterface(Memory * memory, 
        TransceiverCallback cb, 
        void * trans, 
        const uint16_t crc_table[]):
    Module(memory, cb, trans),
    crc_table(crc_table)
{
}

void ByteInputInterface::process_msg(const uint8_t msg[], size_t len)
{
    /* plus 1 for the message size */
    /* plus 2 for the crc */

    if (len > 0xff)
    {
        ERROR("Message length > 255\n");
        return;
    }
    else if (len == 0)
    {
        ERROR("Message length == 0\n");
        return;
    }

    Block * block = memory->allocate(1 + len + 2);

    if (block)
    {
        float ** iter = block->get_iterator();

        **iter = (float) (len & 0xff);
        block->next();

        for (size_t n = 0; n < len; ++n)
        {
            **iter = (float) msg[n];
            block->next();
        }

        uint16_t crc;

        crc = crc_16(crc_table, msg, len);

        **iter = (float) ( (crc & 0xff00) >> 8 );
        block->next();

        **iter = (float) (crc & 0xff);
        block->next();

        block = process(block);
        handoff(block, 0);
    }
    else 
    {
        ERROR("Could not allocate enough space!\n");
    }
}

void ByteInputInterface::dispatch(RadioMsg * msg)
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
        {
            uint8_t dummy[] = {0};

            process_msg(dummy, 1);
            break;
        }

        default:
            break;
    }
}


Block * ByteInputInterface::process(Block * msg)
{
    static char errors[][100] = {
        {"No error"},
        {"Memory allocate error"},
        {"Iterator error"}};

    int error = 0;
    size_t line = 0;

    int n = 0;
    uint8_t byte = 0;
    Block * bit = NULL;
    float ** msg_iter = NULL;
    float ** bit_iter = NULL;

    /* plus 1 at the front for the msg size */
    /* plus 2 at the end for the CRC */
    size_t len = msg->get_size() * 8;

    bit = memory->allocate(len);

    if (!bit) {
        line = __LINE__;
        error = 1;
        goto fail;
    }

    msg_iter = msg->get_iterator();
    bit_iter = bit->get_iterator();

    /* Add the msg */
    msg->reset();

    do
    {
        byte = (uint8_t) **msg_iter;
        for (n = 0; n < 8; ++n)
        {
            **bit_iter = byte & (1 << n) ? 1.0 : 0.0;
            bit->next();
        }
    } while(msg->next());

    if (msg->next() || bit->next()) 
    {
        line = __LINE__;
        error = 2;
        goto fail;
    }

    msg->free();
    return bit;


fail:
    RED;
    fprintf(stderr, "[%zu]: %s\n", line, errors[error]);
    ENDC;
    msg->free();
    if (bit) {
        bit->free();
    }
    return NULL;
}
