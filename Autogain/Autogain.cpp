#include "Autogain.h"
#include <math.h>

const char __name__[] = "Autogain";

const char * Autogain::name() {
    return __name__;
}


Autogain::Autogain(Memory * memory, Module * next, double fs):
    Module(memory, next),
    autogain_c(10.0, 0.01, 44.1E3),
    autogain_b(10.0, 0.10, 44.1E3),
    autogain_a(10.0, 1.00, 44.1E3)
{
}

Autogain::~Autogain()
{
}

Block * Autogain::process(Block * block)
{
    static char errors[][25] = {
        {"No error"},
        {"Block allocation error"}
    };
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
        **out_iter = autogain_a.work(autogain_b.work(autogain_c.work(**iter * 0.07)));
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

