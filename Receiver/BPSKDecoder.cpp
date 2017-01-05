#include <string.h>
#include "BPSKDecoder.h"
#include "../CostasLoop/CostasLoopBlock.h"
#include "../CostasLoop/RC_LowPass.h"

#include <math.h>
#define AND &&
#define OR ||

BPSKDecoder::BPSKDecoder(Memory * memory, 
        Module * next,
        float fs, 
        float fc,
        bool * prefix,
        size_t prefix_len,
        int cycles_per_bit, 
        bool first_bit,
        float threshold):
    Module(memory, next),
    first_bit(first_bit),
    threshold(threshold)
{
    int k, j;
    prefix_mask = (1 << (prefix_len))-1;

    this->prefix = 0;

    j = 0;
    for (k = prefix_len - 1; k >= 0; --k)
    {
        if (prefix[j]) {
            this->prefix |= (1 << k);
        }
        j += 1;
    }

    /*
    GREEN;
    printf("%s: prefix = \n", __FILE__);
    print_shift_register(this->prefix);
    print_shift_register(prefix_mask);
    ENDC;
    */

    // the time difference between two sampling rates is
    // exact sampling rate fs, ts
    // integer sampling rate fk, tk
    // the time at sample n is
    // ts*n, tk*n
    // the time difference is:
    // ts*n - tk*n = n*(ts-tk)
    // the maximum allowable drift is half a period ts/2
    // the number of samples at which the maximum drift occurs is
    // ts/2 = n*|ts-tk|
    // n = ts/2/|ts-tk|
    // n = ts/(2*|ts-tk|)
    // n = 0.5/|1-tk/ts|

    // ts = 1/fc * cycles_per_bit = cycles_per_bit/fc
    // tk = samples_per_bit / fs
    // tk/ts = samples_per_bit/cycles_per_bit*fc/fs

    float samples_per_bit;
    float tk_ts;
    float n;

    samples_per_bit = round(fs/fc*cycles_per_bit);

    tk_ts = samples_per_bit/((float)cycles_per_bit)*fc/fs;

    if (fabs(1 - tk_ts) > 1E-6) {
        n = 0.5/fabs(1 - tk_ts);
        WARNING("Sampling clock drift warning! n = %.3f\n", n);
        WARNING("It is recommended you change fc\n");
    }
    sample_period = (int) samples_per_bit;

    LOG("samples per bit: %d\n", sample_period);

    this->fs = fs;

    trail_samples = 0;
    trail_samples_len = sample_period > 32 ? 32 : sample_period;
}

const char __name__[] = "BPSKDecoder";

const char * BPSKDecoder::name()
{
    return __name__;
}

bool BPSKDecoder::majority_vote()
{
    int n;
    int high = 0;
    int low = 0;

    for (n = 0; n < trail_samples_len; ++n) 
    {
        if (trail_samples & (1 << n)) 
        {
            high += 1;
        }
        else 
        {
            low += 1;
        }
    }
    if (high > low) 
    {
        //LOG("[+%d/-%d]\n", high, low);
        return true;
    }
    else 
    {
        //LOG("[+%d/-%d]\n", high, low);
        return false;
    }
}

void BPSKDecoder::add_level(bool level) 
{
    trail_samples <<= 1;
    trail_samples += level ? 1 : 0;
}

void BPSKDecoder::print_shift_register(uint32_t shift_register)
{
    uint32_t reg = shift_register;

    LOG(" ");
    for (int n = 0; n < 32; ++n)
    {
        if (reg & (1 << n)) 
            printf("1 ");
        else
            printf("0 ");
    }
    printf("\n");
}

Block * BPSKDecoder::process(Block * block)
{
    static RC_LowPass timer(0.25, fs);
    static uint32_t shift_register = 0;
    static float last = 0;
    static int count = 0;
    static bool last_bit = false;
    static enum {ACQUIRE, LOOK_FOR_HEADER, READ_SIZE, COLLECT_BITS} state = ACQUIRE;
    static uint8_t k = 0;
    static uint8_t byte = 0;
    static Block * msg = NULL;
    static float ** msg_iter = NULL;

    CostasLoopBlock * demod = (CostasLoopBlock *) block;
    float * lock = demod->get_pointer(LOCK_SIGNAL);
    //float * freq = demod->get_pointer(FREQUENCY_EST_SIGNAL);
    float * data = demod->get_pointer(IN_PHASE_SIGNAL);

    Block * reset_signal =  memory->allocate(block->get_size());
    float ** reset_iter = reset_signal->get_iterator();

    float derivative;
    bool bit;

    //Block * deri = memory->allocate(demod->get_size());
    //float ** deri_iter = deri->get_iterator();

    demod->reset();

    // temporary
    // state = ACQUIRE;

    do 
    {
        if (*lock < 0.8) 
        {
            if (state != ACQUIRE) 
            {
                /*
                MAGENTA;
                printf("%s: Costas loop unlocked!\n", __FILE__);
                ENDC;
                */
                if (msg) 
                {
                    msg->free();
                    msg = NULL;
                    msg_iter = NULL;
                }
                timer.reset();
                state = ACQUIRE;
            }
            /*
            timer.reset();
            **reset_iter = timer.work(0.0);
            */
            **reset_iter = 0.0;
            reset_signal->next();
        }
        else 
        {
            **reset_iter = timer.work(1.0);
            reset_signal->next();
        }

        add_level( (*data > 0.0) ? true : false );
        
        derivative = (*data - last);
        last = *data;
        //**deri_iter = derivative;
        //**deri_iter = *data;
        //deri->next();

        switch (state)
        {
            case ACQUIRE:
                if (*lock >= 0.8 && fabs(derivative) > threshold) 
                {
                    /*
                    GREEN;
                    printf("%s: state = LOOK_FOR_HEADER\n", __FILE__);
                    ENDC;
                    */
                    state = LOOK_FOR_HEADER;
                    count = 0;
                    last_bit = false;
                    shift_register = 0;
                }
                break;

            case LOOK_FOR_HEADER:
                count += 1;
                if (count >= sample_period)
                {
                    bit = majority_vote();
                    shift_register <<= 1;

                    if (bit != last_bit) 
                    {
                        //printf("1 ");
                        shift_register |= 1;
                        timer.reset();
                    }
                    /*
                    else {
                        printf("0 ");
                    }
                    */

                    last_bit = bit;
                    count = 0;

                    /*
                    printf("%s: shift : ", __FILE__);
                    print_shift_register(shift_register);
                    
                    printf("%s: prefix: ", __FILE__);
                    print_shift_register(prefix);
                    */

                    if ((shift_register & prefix_mask) == prefix) 
                    {
                        LOG("Found prefix!\n");
                        k = 0;
                        byte = 0;
                        state = READ_SIZE;
                    }
                }
                break;

            case READ_SIZE:
                count += 1;
                if (count >= sample_period)
                {
                    bit = majority_vote();
                    if (bit != last_bit)
                    {
                        byte |= (1 << k);
                        timer.reset();
                    }
                    k += 1;

                    last_bit = bit;
                    count = 0;

                    if (k >= 8) 
                    {
                        LOG("Got size: %hhd\n", byte);
                        msg = memory->allocate(byte);
                        msg_iter = msg->get_iterator();
                        k = 0;
                        byte = 0;
                        state = COLLECT_BITS;
                    }
                }
                break;
                

            case COLLECT_BITS:
                count += 1;
                if (count >= sample_period)
                {
                    bit = majority_vote();
                    if (bit != last_bit)
                    {
                        byte |= (1 << k);
                        timer.reset();
                        //printf("1 ");
                    }
                    else {
                        //printf("0 ");
                    }
                    k += 1;

                    last_bit = bit;
                    count = 0;

                    if (k >= 8)
                    {
                        /*
                        GREEN;
                        printf("%c", byte);
                        ENDC;
                        */

                        **msg_iter = byte;
                        if (!msg->next()) 
                        {
                            // print the message:
                            LOG("Received: ");
                            GREEN;
                            msg->reset();
                            char c;
                            do 
                            {
                                c = (char) **msg_iter;
                                if (c == '\n') 
                                {
                                    printf("\\n");
                                }
                                printf("%c", c);

                            } while(msg->next());
                            printf("\n");
                            ENDC;

                            // finish the timer iterator
                            do
                            {
                                **reset_iter = 0.0;
                            } while(reset_signal->next());

                            demod->free();
                            state = ACQUIRE;
                            Block * ret = msg;
                            msg = NULL;
                            msg_iter = NULL;
                            ret->free();
                            //return ret;
                            return reset_signal;
                        }

                        byte = 0;
                        k = 0;
                    }
                }
                break;

            default:
                break;
        }
    } while(demod->next());
    demod->free();
    return reset_signal;
}

