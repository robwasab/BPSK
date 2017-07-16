#include <string.h>
#include "BPSKDecoder.h"
#include "../CostasLoop/CostasLoopBlock.h"
#include "../CostasLoop/RC_LowPass.h"

#include <math.h>
#define AND &&
#define OR ||

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

class HighPass
{
public:
    HighPass(float tau, float fs)
    {
        k = tau * fs / (tau * fs + 1);
        in_last = 0.0;
        out_last = 0.0;
    }

    float work(float in) {
        float out;
        out = k * (in - in_last + out_last);
        out_last = out;
        in_last = in;
        return out;
    }

    double value() {
        return out_last;
    }

    void reset() {
        in_last = 0.0;
        out_last = 0.0;
    }

private:
    float k;
    float in_last;
    float out_last;
};

#define RESET_SIG_DB
//#define HIGH_PASS_DB

Block * BPSKDecoder::process(Block * block)
{
    static HighPass filter(0.005, fs);
    static RC_LowPass timer(0.25, fs);
    static RC_LowPass no_lock_timer(0.25, fs);
    static uint32_t shift_register = 0;
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

#ifdef RESET_SIG_DB
    Block * reset_signal =  memory->allocate(block->get_size());
    float ** reset_iter = reset_signal->get_iterator();
#elif defined(HIGH_PASS_DB)
    Block * hp = memory->allocate(demod->get_size());
    float ** hp_iter = hp->get_iterator();
#endif

    float ac_couple;
    bool bit;


    demod->reset();

    do 
    {
        if (*lock < 0.8) 
        {
            if (state != ACQUIRE) 
            {
                if (msg) 
                {
                    msg->free();
                    msg = NULL;
                    msg_iter = NULL;
                }
                timer.reset();
                LOG("Lost lock. state->ACQUIRE\n");
                state = ACQUIRE;
            }
            timer.reset();
            no_lock_timer.work(1.0);

            if (no_lock_timer.value() > 0.99) 
            {
                LOG("Hard resetting costas loop...\n");
                demod->hard_reset();
                no_lock_timer.reset();
            }
#ifdef RESET_SIG_DB
            **reset_iter = timer.work(0.0);
            **reset_iter = 0.0;
            reset_signal->next();
#endif
        }
        else 
        {
            no_lock_timer.reset();
            if (state != ACQUIRE) 
            {
                timer.work(1.0);

                if (timer.value() > 0.99) {
                    state = ACQUIRE;
                    LOG("watchdog timer. state->ACQUIRE\n");
                    timer.reset();
                }
            }
            else {
                timer.reset();
            }
#ifdef RESET_SIG_DB
            **reset_iter = timer.value();
            reset_signal->next();
#endif
        }

        add_level( (*data > 0.0) ? true : false );
        
        ac_couple = filter.work(*data);

#ifdef HIGH_PASS_DB
        **hp_iter = ac_couple;
        hp->next();
#endif

        switch (state)
        {
            case ACQUIRE:
                if (*lock >= 0.8 && fabs(ac_couple) > threshold) 
                {
                    LOG("Detected start of packet. state->LOOK_FOR_HEADER\n");
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
                        LOG("Found prefix! state->READ_SIZE\n");
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
                        LOG("Got size: %hhu state->COLLECT_BITS\n", byte);
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

#if defined(RESET_SIG_DB)
                            do
                            {
                                **reset_iter = 0.0;
                            } while(reset_signal->next());
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
                            demod->free();
                            ret->free();
                            return reset_signal;
#elif defined(HIGH_PASS_DB)
                            demod->free();
                            ret->free();
                            return hp;
#else
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
    } while(demod->next());

#if defined(RESET_SIG_DB)
    demod->free();
    return reset_signal;

#elif defined(HIGH_PASS_DB)
    demod->free();
    return hp;
#else
    return demod;
#endif
}

