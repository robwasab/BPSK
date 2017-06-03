#include <math.h>
#include "QPSK_Encode.h"

QPSK_Encode::QPSK_Encode(Memory * memory, 
        Module * next, 
        float fs, 
        float fc, 
        int cycles_per_sym,
        int training_cycles):
    Module(memory, next),
    fs(fs),
    fc(fc),
    cycles_per_sym(cycles_per_sym),
    training_cycles(training_cycles)
{
    inc = 2.0*M_PI*fc/fs;
    phase    = 0;
    qpsk_sym = 0;
    samples_per_cycle = fs/fc;
    phase_per_sym = cycles_per_sym * 2.0 * M_PI;

    double bw = fc/cycles_per_sym * 2.0;
    LOG("Estimated bandwidth: %.3f\n", bw);
}

class QPSK_EncodeBlock : public Block
{
public:
    QPSK_EncodeBlock(Block * syms, 
            double * phase,
            int    * qpsk_sym,
            int training_cycles,
            int cycles_per_sym,
            double fc,
            double fs):
        syms(syms),
        phase(phase),
        qpsk_sym(qpsk_sym),
        training_cycles(training_cycles),
        HALF_PI(M_PI/2.0)
    {
        samples_per_cycle = fs/fc;

        len = (size_t) ((training_cycles + syms->get_size()) * cycles_per_sym * samples_per_cycle);
        phase_per_sym = cycles_per_sym * 2.0 * M_PI;
        inc = 2.0 * M_PI * fc / fs;

        ptr = &value;
        qpsk_pha = sym2phase(*qpsk_sym);
        *phase += inc;
        value = (float) sin(*phase + qpsk_pha);
        reset();
        syms_iter = syms->get_iterator();
    }

    double sym2phase(int qpsk_int)
    {
        return qpsk_int * HALF_PI;
    }

    ~QPSK_EncodeBlock() {}

    void free() {
        syms->free();
        delete this;
    }

    bool is_free() {
        return syms->is_free();
    }

    size_t get_size() {
        return len;
    }

    void reset() {
        syms->reset();
        // this forces it to flip on the first bit
        //last_bit = (**bits_iter < 1.0) ? 1.0 : 0.0;
        state = TRAIN;
        k = 1;
        n = 1;
    }

    bool next() 
    {
        if (*phase > phase_per_sym) 
        {
            *phase -= phase_per_sym;

            if (n < len) 
            {
                if (k >= training_cycles) 
                {
                    state = LOAD_SYM;
                }

                switch (state)
                {
                    case TRAIN:
                        k += 1;
                        break;

                    case LOAD_SYM:
                        sym = **syms_iter;

                        if (sym > 1E-5) 
                        {
                            *qpsk_sym += (int)sym;
                            *qpsk_sym %= 4;
                            qpsk_pha = sym2phase(*qpsk_sym);
                        }

                        syms->next();
                        break;
                }
            }
        }

        if (n < len) 
        {
            *phase += inc;
            value = (float) sin(*phase + qpsk_pha);
            n += 1;
            return true;
        }
        else 
        {
            *phase += inc;
            value = (float) sin(*phase + qpsk_pha);
            return true;
        }
    }

    float ** get_iterator() {
        return &ptr;
    }

    void print() {
        printf("QPSK Encode Block\n");
        syms->print();
    }

private:
    Block  * syms;
    double * phase;
    int    * qpsk_sym;

    unsigned int training_cycles;
    double HALF_PI;
    double phase_per_sym;
    size_t len;
    double samples_per_cycle;
    float * ptr;
    float value;
    double inc;
    double qpsk_pha;
    float ** syms_iter;
    enum {TRAIN, LOAD_SYM} state;
    unsigned int k,n;
    float sym;
};

Block * QPSK_Encode::process(Block * syms)
{
    Block * ret = 
     new QPSK_EncodeBlock(syms, &phase, &qpsk_sym, training_cycles, cycles_per_sym, fc, fs);
    return ret;
}

/*
Block * BPSK::process(Block * bits)
{
    static double phase = 0.0;
    static bool inv = false;

    float last_bit = 0.0;
    char errors[][32] =
    {{"No error"},
     {"Allocate error"}};
    int error = 0;
    float ** signal_iter = NULL;
    float ** bits_iter = NULL;
    enum {TRAIN, MODULATE} state;
    state = TRAIN;

    bool quit = false;
    int k;
    float bit;

    size_t len = (size_t)
        ((training_bits + bits->get_size()) * cycles_per_bit * samples_per_cycle);

    Block * signal = memory->allocate(len);

    if (!signal) {
        error = 1;
        goto fail;
    }

    signal_iter = signal->get_iterator();
    bits_iter = bits->get_iterator();

    bits->reset();

    // this forces it to flip on the first bit;
    last_bit = **bits_iter == 0.0 ? 1.0 : 0.0;
    k = 0;

    do
    {
        if (k >= training_bits) {
            state = MODULATE;
        }

        switch (state)
        {
            case TRAIN:
            k += 1;
            break;

            case MODULATE:
            bit = **bits_iter;

            if (last_bit != bit) {
                inv ^= true;
                last_bit = bit;
            }

            if (!bits->next()) {
                quit = true;
            }
            break;
        }

        do 
        {
            **signal_iter = (float) ((inv ? -1.0 : 1.0) * sin(phase));
            phase += inc;
        } while (signal->next() && phase < phase_per_bit); 

        phase -= phase_per_bit;

    } while(!quit);

    bits->free();
    return signal;

fail:
    RED;
    printf("%s\n", errors[error]);
    ENDC;
    bits->free();
    if (signal) {
        signal->free();
    }
    return NULL;
}
*/

const char __name__[] = "QPSK Encoder";

const char * QPSK_Encode::name()
{
    return __name__;
}
