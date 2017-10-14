#include <math.h>
#include "BPSK.h"

BPSK::BPSK(Memory * memory, 
        TransceiverCallback cb,
        void * tran,
        float fs, 
        float fc, 
        int cycles_per_bit,
        int training_bits):
    Module(memory, cb, tran),
    fs(fs),
    fc(fc),
    cycles_per_bit(cycles_per_bit),
    training_bits(training_bits)
{
    inv = 1.0;
    inc = 2.0*M_PI*fc/fs;
    phase = 0;
    one_eighty = M_PI;
    samples_per_cycle = fs/fc;
    phase_per_bit = cycles_per_bit * 2.0 * M_PI;

    double bw = fc/cycles_per_bit * 2.0;
    LOG("* BPSK Encoder Settings:\n");
    LOG("* sampling rate     : %.0lf\n", fs);
    LOG("* center frequency  : %.0lf\n", fc);
    LOG("* samples per cycle : %f\n", samples_per_cycle);
    LOG("* cycles per bit    : %d\n", cycles_per_bit);
    LOG("Estimated bandwidth: %.3f\n", bw);

}

class BPSKBlock : public Block
{
public:
    BPSKBlock(Block * bits, 
            BPSK * bpsk):
        bits(bits),
        bpsk(bpsk)
    {
        len = (size_t) round(
            (bpsk->training_bits + bits->get_size()) * bpsk->cycles_per_bit * bpsk->samples_per_cycle);

        ptr = &value;
        value = (float) ((bpsk->inv) * sin(bpsk->phase));
        //bpsk->phase += bpsk->inc;

        bits_iter = bits->get_iterator();
        notify_sent = false;
        bits->reset();
        state = TRAIN;
        k = 1;
        n = 1;
    }

    ~BPSKBlock() {}

    void free() {
        bits->free();
        delete this;
    }

    bool is_free() {
        return bits->is_free();
    }

    size_t get_size() {
        return len;
    }

    void reset() {
        bits->reset();
        // this forces it to flip on the first bit
        //last_bit = (**bits_iter < 1.0) ? 1.0 : 0.0;
        state = TRAIN;
        k = 1;
        n = 1;
        
        value = (float) ((bpsk->inv) * sin(bpsk->phase));
        bpsk->phase += bpsk->inc;
    }

    bool next() 
    {
        if (bpsk->phase > bpsk->phase_per_bit) 
        {
            bpsk->phase -= bpsk->phase_per_bit;

            if (n < len) 
            {
                if (k >= bpsk->training_bits) 
                {
                    state = LOAD_BIT;
                }

                switch (state)
                {
                    case TRAIN:
                        k += 1;
                        break;

                    case LOAD_BIT:
                    {
                        float bit;
                        bit = **bits_iter;

                        if (bit) 
                        {
                            bpsk->inv *= -1.0;
                        }

                        bits->next();
                        break;
                    }
                }
            }
        }

        if (n < len) 
        {
            value = (float) ((bpsk->inv) * sin(bpsk->phase));
            bpsk->phase += bpsk->inc;
            n += 1;
            return true;
        }
        else 
        {
            value = (float) ((bpsk->inv) * sin(bpsk->phase));
            bpsk->phase += bpsk->inc;

            if (!notify_sent)
            {
                notify_sent = true;
                RadioMsg msg(NOTIFY_MSG_EXHAUSTED);
                bpsk->broadcast(&msg);
            }
            return true;
        }
    }

    float ** get_iterator() {
        return &ptr;
    }

    void print() 
    {
        printf("BPSK Block {\n");
        bits->print();
        printf("}\n");
    }

private:
    Block * bits;
    BPSK * bpsk;
    float * ptr;
    size_t len;
    float value;
    float ** bits_iter;
    enum {TRAIN, LOAD_BIT} state;
    unsigned int k,n;
    bool notify_sent;
};

Block * BPSK::process(Block * bits)
{
    Block * ret = new BPSKBlock(bits, this);
    return ret;
}

const char __name__[] = "BPSK";

const char * BPSK::name()
{
    return __name__;
}

