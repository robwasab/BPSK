#include <math.h>
#include "PSK8_SigGen.h"

PSK8_SigGen::PSK8_SigGen(Memory * memory, 
        TransceiverCallback cb,
        void * trans,
        float fs, 
        float fc, 
        int cycles_per_sym,
        int training_cycles):
    Module(memory, cb, trans),
    fs(fs),
    fc(fc),
    cycles_per_sym(cycles_per_sym),
    training_cycles(training_cycles)
{
    phase    = 0;
    psk8_phase = 0.0;
    psk8_ampli = 1.0;

    double bw = fc/cycles_per_sym * 2.0;
    LOG("Estimated bandwidth: %.3f\n", bw);
}

class PSK8_SigGenBlock : public Block
{
public:
    PSK8_SigGenBlock(Block * syms, 
            double * phase,
            double * psk8_phase,
            double * psk8_ampli,
            int training_cycles,
            int cycles_per_sym,
            double fc,
            double fs):
        syms(syms),
        phase(phase),
        psk8_phase(psk8_phase),
        psk8_ampli(psk8_ampli),
        training_cycles(training_cycles),
        HALF_PI(M_PI/2.0)
    {
        samples_per_cycle = fs/fc;

        len = (size_t) ((training_cycles + syms->get_size()) * cycles_per_sym * samples_per_cycle);
        phase_per_sym = cycles_per_sym * 2.0 * M_PI;
        inc = 2.0 * M_PI * fc / fs;

        ptr = &value;

        reset();

        calculate();

        syms_iter = syms->get_iterator();
    }

    void calculate()
    {
        value = (float) ( *psk8_ampli * sin(*phase + *psk8_phase) );
    }

    void decode(int symbol)
    {
        *psk8_phase += HALF_PI * (symbol & 0x03);
        if (*psk8_phase >= 360.0)
        {
            *psk8_phase -= 360.0;
        }

        *psk8_ampli += 2.0 * (symbol & (1 << 3));
        if (*psk8_ampli >= 4.0)
        {
            *psk8_ampli -= 4.0;
        }
    }

    ~PSK8_SigGenBlock() {}

    void free() 
    {
        syms->free();
        delete this;
    }

    bool is_free() 
    {
        return syms->is_free();
    }

    size_t get_size() 
    {
        return len;
    }

    void reset() 
    {
        syms->reset();
        state = TRAIN;

        /* prepare for training */
        *psk8_ampli = 1.0;

        k = 0;
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
                    {
                        float sym;
                        sym = **syms_iter;

                        if (sym > 1E-5) 
                        {
                            decode( (int) sym );
                        }

                        syms->next();
                        break;
                    }
                }
            }
        }

        if (n < len) 
        {
            *phase += inc;
            calculate();
            n += 1;
            return true;
        }
        else 
        {
            *phase += inc;
            calculate();
            return true;
        }
    }

    float ** get_iterator() 
    {
        return &ptr;
    }

    void print() 
    {
        printf("PSK8 Encode Block\n");
        syms->print();
    }

private:
    Block  * syms;
    double * phase;
    double * psk8_phase;
    double * psk8_ampli;

    unsigned int training_cycles;
    double HALF_PI;
    double phase_per_sym;
    size_t len;
    double samples_per_cycle;
    float * ptr;
    float value;
    double inc;
    float ** syms_iter;
    enum {TRAIN, LOAD_SYM} state;
    unsigned int k,n;
};

Block * PSK8_SigGen::process(Block * syms)
{
    Block * ret = 
        new PSK8_SigGenBlock(syms, &phase, &psk8_phase, &psk8_ampli, training_cycles, cycles_per_sym, fc, fs);
    return ret;
}

void PSK8_SigGen::dispatch(RadioMsg * msg)
{
    Module::dispatch(msg);

    switch(msg->type)
    {
        case CMD_TEST_PSK8_SIG_GEN:
            LOG("Testing PSK8 Signal Generator!\n");
            break;
        default:
            break;
    }
}

const char __name__[] = "PSK8_SigGen";

const char * PSK8_SigGen::name()
{
    return __name__;
}

