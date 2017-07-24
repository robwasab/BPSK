#include <string.h>
#include "BPSKDecoder.h"
#include "../CostasLoop/CostasLoopBlock.h"
#include "../CostasLoop/RC_LowPass.h"

#include <math.h>
#define AND &&
#define OR ||

char state_names[][64] =
{
    [ACQUIRE] = "ACQUIRE",
    [LOOK_FOR_HEADER] = "LOOK_FOR_HEADER",
    [READ_SIZE] = "READ_SIZE",
    [COLLECT_BITS] = "COLLECT_BITS",
};

BPSKDecoder::BPSKDecoder(Memory * memory, 
        TransceiverCallback cb,
        void * trans,
        float fs, 
        float fc,
        bool * prefix,
        size_t prefix_len,
        int cycles_per_bit, 
        float threshold):
    Module(memory, cb, trans),
    threshold(threshold),
    state(ACQUIRE),
    msg(NULL),
    timer(0.25, fs),
    filter(0.005, fs),
    shift_register(0),
    count(0),
    last_bit(false),
    k(0),
    byte(0),
    msg_iter(NULL),
    byte_msg()
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

    LOG("* BPSK Decoder Settings:\n");
    LOG("*   sampling rate : %.0lf\n", fs);
    LOG("*   center freq   : %.0lf\n", fc);
    LOG("*   prefix_len    : %zu\n", prefix_len);
    LOG("*   cycles_per_bit: %d\n", cycles_per_bit);

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

#define RESET_SIG_DB
//#define HIGH_PASS_DB

bool isChar(char c, const char check[])
{
    int k;
    for (k = 0; check[k] != '\0'; k++)
    {
        if (c == check[k])
        {
            return true;
        }
    }
    return false;
}

void BPSKDecoder::dispatch(RadioMsg * radio_msg)
{
    RadioData * data;
    Block * block;

    data = (RadioData *) radio_msg;

    switch(radio_msg->type)
    {
        case PROCESS_DATA:
            block = data->get_block();

            block = process(block);

            if (block != NULL)
            {
                handoff(block, data->get_tid());
            }
            break;

        case NOTIFY_PLL_RESET:
            break;

        case NOTIFY_PLL_LOCK:
            break;

        case NOTIFY_PLL_LOST_LOCK:
            if (state != ACQUIRE) 
            {
                if (msg) 
                {
                    msg->free();
                    msg = NULL;
                    msg_iter = NULL;
                }
                timer.reset();
                LOG("Lost lock. %s->ACQUIRE\n", state_names[state]);
                state = ACQUIRE;
            }
            timer.reset();
            break;

        default:
            break;
    }
}

void BPSKDecoder::plot_debug_signal(float signal)
{

}

Block * BPSKDecoder::process(Block * block)
{
    #ifdef RESET_SIG_DB
    Block * reset_signal =  memory->allocate(block->get_size());
    float ** reset_iter = reset_signal->get_iterator();
    #endif

    #ifdef HIGH_PASS_DB
    Block * hp = memory->allocate(block->get_size());
    float ** hp_iter = hp->get_iterator();
    #endif

    float ac_couple; /* High Pass filter variable */
    float ** iter;   /* block iterator */
    bool bit;

    iter = block->get_iterator();

    block->reset();

    do 
    {

        if (state != ACQUIRE) 
        {
            timer.work(1.0);

            if (timer.value() > 0.99) 
            {
                LOG("watchdog timer. %s->ACQUIRE\n", state_names[state]);
                state = ACQUIRE;
                timer.reset();
            }
        }
        else 
        {
            timer.reset();
        }
        #ifdef RESET_SIG_DB
        **reset_iter = timer.value();
        reset_signal->next();
        #endif

        add_level( (**iter > 0.0) ? true : false );
        
        ac_couple = filter.work(**iter);

        #ifdef HIGH_PASS_DB
        **hp_iter = ac_couple;
        hp->next();
        #endif

        switch (state)
        {
            case ACQUIRE:
                if (fabs(ac_couple) > threshold) 
                {
                    LOG("Detected start of packet. %s->LOOK_FOR_HEADER\n",
                            state_names[state]);
                    timer.reset();
                    state = LOOK_FOR_HEADER;
                    count = 0;
                    last_bit = majority_vote();
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
                        shift_register |= 1;
                        timer.reset();
                    }

                    last_bit = bit;
                    count = 0;

                    /*
                    LOG("shi: ");
                    print_shift_register(shift_register);
                    
                    LOG("pre: ");
                    print_shift_register(prefix);
                    */

                    if ((shift_register & prefix_mask) == prefix) 
                    {
                        LOG("Found prefix! %s->READ_SIZE\n", state_names[state]);
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
                        LOG("Got size: %hhu + CRC\n", byte);
                        LOG("%s->COLLECT_BITS\n",state_names[state]);
                        msg = memory->allocate(byte + 2);
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
                    }
                    k += 1;

                    last_bit = bit;
                    count = 0;

                    if (k >= 8)
                    {
                        **msg_iter = byte;

                        if (!msg->next()) 
                        {
                            /* Read all the bytes we are done! */

                            memset(byte_msg, 0, sizeof(byte_msg));

                            /* Print the message */
                            LOG("Received: ");
                            GREEN;
                            msg->reset();
                            
                            char c;
                            int index = 0;

                            do 
                            {
                                c = (char) **msg_iter;
                                byte_msg[index++] = (uint8_t) c;

                                /* Print the message in a nice format */
                                if (c == '\n') 
                                {
                                    printf("\\n");
                                }
                                else if ((c >= 'a' && c <= 'z') ||
                                         (c >= 'A' && c <= 'Z'))
                                {
                                    printf("%c", c);
                                }
                                else if (isChar(c, " !@#$%^&*()_-+={}[]|:;<>,.?/""'"))
                                {
                                    printf("%c", c);
                                }
                                else if (c == '\0')
                                {
                                    printf("█");
                                }
                                else
                                {
                                    printf("?");
                                }

                            } while(msg->next());
                            printf("\n");

                            ENDC;

                            /* finish off the reset signal with zeros */
                            #if defined(RESET_SIG_DB)
                            do
                            {
                                **reset_iter = 0.0;
                            } while(reset_signal->next());

                            /* finish off the high pass signal with zeros */
                            #elif defined(HIGH_PASS_DB)
                            do
                            {
                                **hp_iter = 0.0;
                            } while(hp->next());
                            #endif

                            state = ACQUIRE;
                            Block * ret = msg;
                            msg = NULL;
                            msg_iter = NULL;

                            #if defined(RESET_SIG_DB)
                            block->free();
                            ret->free();
                            return reset_signal;

                            #elif defined(HIGH_PASS_DB)
                            block->free();
                            ret->free();
                            return hp;
                            #else

                            block->free();
                            return ret;
                            #endif
                        }

                        byte = 0;
                        k = 0;
                    }
                }
                break;

            default:
                break;
        }
    } while(block->next());

    #if defined(RESET_SIG_DB)
    block->free();
    return reset_signal;

    #elif defined(HIGH_PASS_DB)
    block->free();
    return hp;
    #else
    block->free();
    return NULL;
    #endif
}

