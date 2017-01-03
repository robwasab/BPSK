#include <stdint.h>
#include <stdio.h>
#include "generator.h"

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

#define RAND_LEN ((1 << ML_BITS) - 1)

bool shift_register[ML_BITS];
bool sequence[RAND_LEN];

void generate_ml_sequence(size_t * len, bool ** ret_seq)
{
    int n,k;
    bool bit;

    for (n = 0; n < ML_BITS; ++n) 
    {
        shift_register[n] = true;
    }

    printf("%s: Calculating pseudo random seq of len: %d\n", __FILE__, RAND_LEN);

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

        sequence[n] = bit;
        printf("%d ", bit);
    }
    printf("\n");

    *len = RAND_LEN;
    *ret_seq = sequence;
}
