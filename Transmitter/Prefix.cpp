#include "Prefix.h"

int feedback_table[] = 
#if ML_BITS == 2
    {+1,+0,-1,};
#elif ML_BITS == 3
    {+2,+1,-1,};
#elif ML_BITS == 4
    {+3,+2,-1};
#elif ML_BITS == 5
    {+4,+2,-1};
#elif ML_BITS == 6
    {+5,+4,-1};
#elif ML_BITS == 7
    {+6,+5,-1};
#elif ML_BITS == 8
    {+7,+5,+4,+3,-1};
#elif ML_BITS == 9
    {+8,+4,-1};
#elif ML_BITS == 10
    {+9,+6,-1};
#elif ML_BITS == 11
    {10,+8,-1};
#elif ML_BITS == 12
    {11,10,+9,+3,-1};
#elif ML_BITS == 13
    {12,11,10,+7,-1};
#elif ML_BITS == 14
    {13,12,11,+1,-1};
#elif ML_BITS == 15
    {14,13,-1};
#elif ML_BITS == 16
    {15,14,12,+3,-1};
#elif ML_BITS == 17
    {16,13,-1};
#elif ML_BITS == 18
    {17,10,-1};
#elif ML_BITS == 19
    {18,17,16,13,-1};
#else
#error unknown ML_BITS
#endif

Prefix::Prefix(Memory * memory, 
            Module * next):
    Module(memory, next)
{
    int n,k;
    bool bit;

    for (n = 0; n < ML_BITS; ++n) 
    {
        shift_register[n] = true;
    }

    printf("Calculating pseudo random sequence len: %d\n", RAND_LEN);

    for (n = 0; n < RAND_LEN; ++n)
    {
        bit = false;
        for (k = 0; feedback_table[k] != -1; ++k)
        {
            bit ^= shift_register[feedback_table[k]];
        }

        for (k = ML_BITS-1; k > 0; --k)
        {
            shift_register[k] = shift_register[k-1];
        }

        shift_register[0] = bit;

        rand[n] = bit;
        printf("%d ", bit);
    }
    printf("\n");
}

void Prefix::encode_helper(Block * encode, bool inv)
{
    int n;
    float ** iter = encode->get_iterator();
    for (n = 0; n < RAND_LEN; ++n)
    {
        **iter = inv ^ rand[n];
        encode->next();
    }
}

Block * Prefix::process(Block * bits)
{
    float ** bits_iter;
    float ** encode_iter;
    int n,k;
    bool bit;

    /* STEP 1: Prefix the data with maximal length pseudo random header
     * STEP 2: replace each '1' bit with the same pseudo random header,
     *         replace each '0' bit with an inversed header.
     * STORY : Encode each bit with a sudo random sequence, but you must
     *         prefix the data with another sudo random sequence in order to 
     *         mark the start of the packet. We are just reusing the same
     *         pseudo random sequence. 
     */

    //Block * encode = memory->allocate((bits->get_size() + RAND_LEN) * RAND_LEN);
    Block * encode = memory->allocate((bits->get_size() + RAND_LEN));
    
    if (!encode) {
        goto fail;
    }

    bits_iter = bits->get_iterator();
    encode_iter = encode->get_iterator();

    bits->reset();

    for (n = 0; n < RAND_LEN; ++n)
    {
        //encode_helper(encode, rand[n]); 
        **encode_iter = rand[n];
        encode->next();
    }

    /*
    do
    {
        encode_helper(encode, **bits_iter);
    } while(bits->next());
    */

    do
    {
        **encode_iter = **bits_iter;
    } while(encode->next() && bits->next());


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
