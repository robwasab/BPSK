#include <string.h>
#include <stdint.h>
#include "Prefix.h"

Prefix::Prefix(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            bool * prefix,
            size_t prefix_len):
    Module(memory, cb, trans)
{
    this->prefix = new bool[prefix_len];
    memcpy(this->prefix, prefix, prefix_len);
    this->prefix_len = prefix_len;
}

Block * Prefix::process(Block * bits)
{
    float ** bits_iter;
    float ** encode_iter;
    uint8_t size;
    int n;

    // prefix + data + 2 zero bytes
    size = bits->get_size()/8;
    Block * encode = memory->allocate( prefix_len + bits->get_size() + 16);
    
    if (!encode) {
        goto fail;
    }

    bits_iter = bits->get_iterator();
    encode_iter = encode->get_iterator();

    bits->reset();

    for (n = 0; n < prefix_len; ++n)
    {
        **encode_iter = prefix[n];
        encode->next();
    }

    do
    {
        **encode_iter = **bits_iter;
    } while(encode->next() && bits->next());

    for (n = 0; n < 16; ++n)
    {
        **encode_iter = 0.0;
        encode->next();
    }

    bits->free();
    return encode;

fail:
    RED;
    printf("Could not allocate PREFIX block!\n");
    ENDC;
    bits->free();
    if (encode) {
        encode->free();
    }
    return NULL;
}
