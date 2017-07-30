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
            double * phase,
            int training_bits,
            int cycles_per_bit,
            double fc,
            double fs):
        bits(bits),
        phase(phase),
        training_bits(training_bits)
    {
        samples_per_cycle = fs/fc;

        len = (size_t) ((training_bits + bits->get_size()) * cycles_per_bit * samples_per_cycle);
        phase_per_bit = cycles_per_bit * 2.0 * M_PI;
        inc = 2.0*M_PI*fc/fs;

        ptr = &value;
        inv = 1.0;
        *phase += inc;
        value = (float) inv * sin(*phase);
        reset();
        bits_iter = bits->get_iterator();
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
    }

    bool next() 
    {
        if (*phase > phase_per_bit) 
        {
            *phase -= phase_per_bit;

            if (n < len) 
            {
                if (k >= training_bits) 
                {
                    state = LOAD_BIT;
                }

                switch (state)
                {
                    case TRAIN:
                        k += 1;
                        break;

                    case LOAD_BIT:
                        bit = **bits_iter;

                        if (bit) 
                        {
                            inv *= -1.0;
                        }

                        bits->next();
                        break;
                }
            }
        }

        if (n < len) 
        {
            *phase += inc;
            value = (float) inv * sin(*phase);
            n += 1;
            return true;
        }
        else 
        {
            *phase += inc;
            value = (float) inv * sin(*phase);
            return true;
        }
    }

    float ** get_iterator() {
        return &ptr;
    }

    void print() {
        printf("BPSK Block {\n");
        bits->print();
        printf("}\n");
    }

private:
    Block * bits;
    double * phase;
    unsigned int training_bits;
    double phase_per_bit;
    size_t len;
    double samples_per_cycle;
    float * ptr;
    float value;
    double inc;
    double inv;
    //float last_bit;
    float ** bits_iter;
    enum {TRAIN, LOAD_BIT} state;
    unsigned int k,n;
    float bit;
};

Block * BPSK::process(Block * bits)
{
    Block * ret = new BPSKBlock(bits, &phase, training_bits, cycles_per_bit, fc, fs);
    return ret;
}

const char __name__[] = "BPSK";

const char * BPSK::name()
{
    return __name__;
}
