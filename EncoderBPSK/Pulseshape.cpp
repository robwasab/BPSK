#include "Pulseshape.h"
#include "../Colors/Colors.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// Ported from MATLAB Software Receiver Design
// Square Root Raised Cosine waveform
void Pulseshape::srrc(int symbol_delay, float beta, float m)
{
    int start = -symbol_delay * m;
    int stop = 1+symbol_delay * m;
    len = stop - start;
    min = 1E9;
    max = -1E9;
    pulseshape = new float[len];
    history = new float[len];
    memset(history, 0, sizeof(float) * len);

    if (beta == 0.0) {
        beta = 1E-8; // numerical problems if beta = 0
    }

    int n = 0;
    double sqrt_m = sqrt(m);
    double pi = M_PI;

    for (float k = start + 1E-2; k < stop; k += 1.0)
    {
        pulseshape[n] = 
            4.0*beta/sqrt_m*(cos((1.0+beta)*pi*k/m) + sin((1.0-beta)*pi*k/m) /
            (4.0*beta*k/m)) / (pi*(1.0-16.0*pow(beta*k/m,2.0)));
        //printf("ps[%d]: %.3f\n", n, pulseshape[n]);

        if (pulseshape[n] > max) {
            max = pulseshape[n];
        }
        if (pulseshape[n] < min) {
            min = pulseshape[n];
        }
        n += 1;
    }
    //printf("len(ps): %zu\n", len);
}

Pulseshape::Pulseshape(Memory * memory, Module * next, float beta, float M):
    Module(memory, next),
    m(M)
{
    sym_delay = 3;
    srrc(sym_delay, 0.5, m);
    //buffer = float[m];
}

Pulseshape::~Pulseshape()
{
    delete [] pulseshape;
    delete [] history;
    //delete [] buffer;
}

float Pulseshape::calculate()
{
    size_t n;
    float ret = 0;
    for (n = 0; n < len; ++n) 
    {
        ret += pulseshape[n] * history[n];
    }
    return ret;
}

void Pulseshape::slide()
{
    size_t n;
    for (n = len-1; n > 0; --n)
    {
        history[n] = history[n-1];
    }
}

Block * Pulseshape::process(Block * bits)
{
    int n,k;
    char errors[][50] =
    {{"No error"},
     {"Allocation error"} };
    int error = 0;
    
    Block * signal = memory->allocate((bits->get_size() + 2*sym_delay) * m);

    float ** bits_iter = bits->get_iterator();
    float ** signal_iter = signal->get_iterator();

    if (!signal) {
        error = 1;
        goto fail;
    }

    bits->reset();

    for (k = 0; k < sym_delay; ++k) 
    {
        history[0] = **bits_iter ? 1.0 : -1.0;
        bits->next();

        for (n = 0; n < m; ++n)
        {
            **signal_iter = calculate();
            slide();
            history[0] = 0.0;
            signal->next();
        }
    }

    do
    {
        history[0] = **bits_iter ? 1.0 : -1.0;
        
        for (n = 0; n < m; ++n)
        {
            **signal_iter = calculate();
            slide();
            history[0] = 0.0;
            signal->next();
        }

    } while(bits->next());

    for (k = 0; k < sym_delay; ++k) 
    {
        for (n = 0; n < m; ++n)
        {
            **signal_iter = calculate();
            slide();
            history[0] = 0.0;
            signal->next();
        }
    }

    bits->free();

    return signal;

fail:
    RED;
    fprintf(stderr, "%s\n", errors[error]);
    ENDC;
    bits->free();
    if (signal) {
        signal->free();
    }
    return NULL;
}

const char __name__[] = "Pulseshape";

const char * Pulseshape::name()
{
    return __name__;

}
