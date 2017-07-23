#include "ByteInterface.h"

ByteInterface::ByteInterface(Memory * memory, TransceiverCallback cb, void * trans):
    Module(memory, cb, trans)
{

}


Block * ByteInterface::process(Block * msg)
{
    static char errors[][100] = {
        {"No error"},
        {"Memory allocate error"},
        {"Iterator error"} };
    int error = 0;
    size_t line = 0;

    int n = 0;
    uint8_t byte = 0;
    Block * bit = NULL;
    float ** msg_iter = NULL;
    float ** bit_iter = NULL;

    size_t len = msg->get_size() * 8;

    bit = memory->allocate(len);

    if (!bit) {
        line = __LINE__;
        error = 1;
        goto fail;
    }

    msg_iter = msg->get_iterator();
    bit_iter = bit->get_iterator();

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

    if (msg->next() || bit->next()) {
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
