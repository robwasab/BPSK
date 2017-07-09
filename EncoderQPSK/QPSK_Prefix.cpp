#include <string.h>
#include <stdint.h>
#include "QPSK_Prefix.h"
#include "../MaximumLength/generator.h"

QPSK_Prefix::QPSK_Prefix(Memory * memory, 
            TransceiverCallback cb,
            void * trans):
    Module(memory, cb, trans)
{
    size_t prefix_len;
    bool * prefix;
    generate_ml_sequence(&prefix_len, &prefix);

    // The prefix must be even. The maximum length routine always returns an odd length sequence
    // So, allocate memory for the maximum length header plus 1 element
    // set the first element to true, so that it triggers the reciever
    bool * temp = new bool[prefix_len + 1];
    memcpy(temp + 1, prefix, prefix_len);
    temp[0] = true;

    this->qpsk_prefix_len = (prefix_len + 1)/2;
    this->qpsk_prefix = new int[qpsk_prefix_len];

    LOG("Converting maximum length header to qpsk header:\n");
    for (int n = 0; n < prefix_len + 1; n++)
    {
        if (temp[n] == true) { printf("1 "); }
        else { printf("0 "); }
    }
    printf("\n");
    LOG("QPSK header:\n");

    for (int n = 0; n < this->qpsk_prefix_len; n++)
    {
        int val = 0;
        val += temp[n*2]     ? (1 << 0) : 0;
        val += temp[n*2 + 1] ? (1 << 1) : 0;

        this->qpsk_prefix[n] = val;
        printf("%d ", val);
    }
    printf("\n");

    delete [] temp;
}

Block * QPSK_Prefix::process(Block * bits)
{
    int back_porch = 16;
    float ** bits_iter;
    float ** encode_iter;
    uint8_t size;
    int n;


    size = bits->get_size()/4;
    Block * encode = memory->allocate( qpsk_prefix_len + 4 + bits->get_size() + back_porch);
    
    if (!encode) {
        goto fail;
    }

    bits_iter = bits->get_iterator();
    encode_iter = encode->get_iterator();

    bits->reset();

    for (n = 0; n < qpsk_prefix_len; ++n)
    {
        **encode_iter = qpsk_prefix[n];
        encode->next();
    }

    /* Add size of message */
    for (n = 0; n < 4; ++n)
    {
        int val = 0;
        val |= size & (1 << (n*2))     ? 0x01 : 0x00;
        val |= size & (1 << (n*2 + 1)) ? 0x02 : 0x00;
        **encode_iter = (float) val; 
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
