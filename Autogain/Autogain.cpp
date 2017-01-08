#include "Autogain.h"
#include <math.h>

const char __name__[] = "Autogain";

const char * Autogain::name() {
    return __name__;
}

Autogain::Autogain(Memory * memory, Module * next, double fs):
    Module(memory, next)
{
    peak_tau = 0.005;

    //averager
    lowp_tau = 0.05;

    max_gain = 10.0;
    comp_thresh = 1.0;
    gain = max_gain;
    
    //peak_cond = new RC_LowPass(0.001, fs);
    peak = new RC_LowPass(peak_tau, fs);
    lowp = new RC_LowPass(lowp_tau, fs);
    //peak_cond->last = 0.0;
    lowp->last = max_gain;
    peak->last = 0.0;
}

Autogain::~Autogain()
{
    //delete peak_cond;
    delete peak;
    delete lowp;
}

Block * Autogain::process(Block * block)
{
    static char errors[][25] = {
        {"No error"},
        {"Block allocation error"}
    };
    float output = 0.0;
    float rectify = 0.0;
    float peak_out = 0.0;
    int error = 0;
    float ** iter = NULL;
    float ** out_iter = NULL;
    Block * out = NULL;

    iter = block->get_iterator();
    out = memory->allocate(block->get_size());
    
    if (!out) {
        error = 1;
        goto fail;
    }

    out_iter = out->get_iterator();

    block->reset();

    do
    {
        output = **iter * gain;;
        //rectify = peak_cond->work(fabs(output));
        rectify = fabs(output);

        if (rectify > peak->value()) {
            peak->last = rectify;
            peak_out = rectify;
        }
        else {
            peak_out = peak->work(0.0);
        }

        //**out_iter = peak_out;

        if (peak_out < comp_thresh) {
            gain = lowp->work(max_gain);
        }
        else {
            gain = lowp->work(0.0);
        }

        **out_iter = output/5.0;

    } while(out->next() && block->next());

    block->free();
    return out;
fail:
    block->free();
    if (out) {
        out->free();
    }
    ERROR("%s\n", errors[error]);
    return NULL;
}

