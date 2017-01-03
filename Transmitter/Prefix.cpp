#include <string.h>
#include "Prefix.h"

Prefix::Prefix(Memory * memory, 
            Module * next, 
            bool * prefix,
            size_t prefix_len):
    Module(memory, next)
{
    this->prefix = new bool[prefix_len];
    memcpy(this->prefix, prefix, prefix_len);
    this->prefix_len = prefix_len;
}

void Prefix::encode_helper(Block * encode, bool inv)
{
    int n;
    float ** iter = encode->get_iterator();

    for (n = 0; n < prefix_len; ++n)
    {
        **iter = inv ^ prefix[n];
        encode->next();
    }
}

Block * Prefix::process(Block * bits)
{
    float ** bits_iter;
    float ** encode_iter;
    uint8_t size;
    int n;

    /* STEP 1: Prefix the data with maximal length pseudo random header
     * STEP 2: replace each '1' bit with the same pseudo random header,
     *         replace each '0' bit with an inversed header.
     * STORY : Encode each bit with a sudo random sequence, but you must
     *         prefix the data with another sudo random sequence in order to 
     *         mark the start of the packet. We are just reusing the same
     *         pseudo random sequence. 
     */

    //Block * encode = memory->allocate((bits->get_size() + RAND_LEN) * RAND_LEN);

    // prefix + size of data contained in a byte + data + zero byte
    size = bits->get_size()/8;
    Block * encode = memory->allocate( prefix_len + 8 + bits->get_size() + 8);
    
    if (!encode) {
        goto fail;
    }

    bits_iter = bits->get_iterator();
    encode_iter = encode->get_iterator();

    bits->reset();

    for (n = 0; n < prefix_len; ++n)
    {
        //encode_helper(encode, rand[n]); 
        **encode_iter = prefix[n];
        encode->next();
    }

    /*
    do
    {
        encode_helper(encode, **bits_iter);
    } while(bits->next());
    */

    for (n = 0; n < 8; ++n)
    {
        **encode_iter = (size & (1 << n)) ? 1.0 : 0.0;
        encode->next();
    }

    do
    {
        **encode_iter = **bits_iter;
    } while(encode->next() && bits->next());

    for (n = 0; n < 8; ++n)
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
