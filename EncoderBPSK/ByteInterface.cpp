#include "ByteInterface.h"

ByteInterface::ByteInterface(Memory * memory, 
        TransceiverCallback cb, 
        void * trans, 
        const uint16_t crc_table[]):
    Module(memory, cb, trans),
    crc_table(crc_table)
{

}

void ByteInterface::process_msg(const uint8_t msg[], size_t len)
{
    /* plus 1 for the message size */
    /* plus 2 for the crc */

    if (len > 0xff)
    {
        ERROR("Message length > 255\n");
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

        **iter = 0.0f;
        block->next();

        **iter = 0.0f;
        block->next();

        block = process(block);
        handoff(block, 0);
    }
    else 
    {
        ERROR("Could not allocate enough space!\n");
    }
}


Block * ByteInterface::process(Block * msg)
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
