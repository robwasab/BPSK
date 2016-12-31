#include "BandPass.h"
#include <math.h>

#define DEBUG

const char __name__[] = "BandPass";

const char * BandPass::name() {
    return __name__;
}

struct Poles {
	int order;
	int len;
	float Q[9];
};
typedef struct Poles Poles;

Poles q_poles[] = {
{2,  1, {0.71, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}},
{4,  2, {0.54, 1.31, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}},
{6,  3, {0.52, 0.71, 1.93, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}},
{8,  4, {0.51, 0.60, 0.90, 2.56, 0.00, 0.00, 0.00, 0.00, 0.00}},
{10, 5, {0.51, 0.56, 0.71, 1.10, 3.20, 0.00, 0.00, 0.00, 0.00}},
{12, 6, {0.50, 0.54, 0.63, 0.82, 1.31, 3.83, 0.00, 0.00, 0.00}},
{14, 7, {0.50, 0.53, 0.59, 0.71, 0.94, 1.51, 4.47, 0.00, 0.00}},
{16, 8, {0.50, 0.52, 0.57, 0.65, 0.79, 1.06, 1.72, 5.10, 0.00}}};

/*
 * Second order bandpass is described like so:
 * H_bp(s) = s*a/(s^2 + s*b + c)
 * d = 2*fs, fs is the sampling frequency
 *
 * The corresponding discrete transfer function is:
 * ( ad - adz^-2)/((d^2 + bd + c) + (2c-2d^2)z^-1 + (d^2-bd+c)z^-2)
 */
SOSBandPass::SOSBandPass(float a, float b, float c, float d)
{
    float a0 = d*d + b*d + c;
    float a1 = 2*c - 2*d*d;
    float a2 = d*d - b*d + c;
    float b0 = a*d;
    float b1 = 0;
    float b2 =-a*d;

    this->a[0] = 1.0;
    this->a[1] = -a1/a0;
    this->a[2] = -a2/a0;

    this->b[0] = b0/a0;
    this->b[1] = b1/a0;
    this->b[2] = b2/a0;

    this->y[0] = 0.0;
    this->y[1] = 0.0;
    this->y[2] = 0.0;

    this->x[0] = 0.0;
    this->x[1] = 0.0;
    this->x[2] = 0.0;

    #ifdef DEBUG
    for (int n = 0; n < 3; n++)
    {
            printf("a[%d] %.6f\t", n, this->a[n]); 
    }
    printf("\n");

    for (int n = 0; n < 3; n++)
    {
            printf("b[%d] %.6f\t", n, this->b[n]); 
    }
    printf("\n");
    #endif
}

float SOSBandPass::work(float sig)
{
    x[0] = sig;
    // b[1] = 0, so don't worry about it
    y[0] = (x[2] * b[2]) + (x[0] * b[0]) + (y[2] * a[2]) + (y[1] * a[1]);
    
    x[2] = x[1];
    x[1] = x[0];
    y[2] = y[1];
    y[1] = y[0];
    return y[0];
}

void SOSBandPass::reset()
{
    x[0] = 0.0;
    x[1] = 0.0;
    x[2] = 0.0;
    
    y[0] = 0.0;
    y[1] = 0.0;
    y[2] = 0.0;
}

float warp(float fc, float fs)
{
	return 2.0*fs*tan(0.5*2.0*M_PI*fc/fs);
}


BandPass::~BandPass()
{
    if (filters) {
        for (int n = 0; n < order; n++)
        {
                delete filters[n];
        }
        delete [] filters;
    }
}

BandPass::BandPass(Memory * memory, 
        Module * next, 
        float fs,
        float fc, 
        float bw, 
        int order):
    Module(memory, next),
    fs(fs),
    fc(fc),
    bw(bw),
    order(order)
{
    if (order % 2 || order < 2) 
    {
        fprintf(stderr, "%s(): order must be even!\n", __func__);
        fprintf(stderr, "%s(): setting order to 2...\n", __func__);
        order = 2;
    }

    float wc = warp(fc, fs);
    float w1 = warp(fc - bw/2.0, fs);
    float w2 = warp(fc + bw/2.0, fs);
    float Q = wc/(w2 - w1);
    float D = 2.0*fs;

    filters = new SOSBandPass*[order];

    float q_lp, a, b, q, wo1, wo2, A1, B1, C1, A2, B2, C2;

    Poles poles = q_poles[order/2-1];

    int k = 0;

    for (int n = 0; n < poles.len; n++)
    {
            q_lp = poles.Q[n];

            a = 1.0/q_lp;
            b = 1.0;
            q = sqrt(Q/a * ((2*Q/a + b/(2*a*Q)) + 
                    sqrt( pow((2*Q/a + b/(2*a*Q)), 2) - 1)));
            wo1 = a*q/(2*Q) + 0.5*sqrt(b/pow(Q, 2) - 1/pow(q, 2));
            wo2 = 1.0/wo1;

            A1 = 1.0/Q * wc;
            B1 = wo1/q * wc;
            C1 = wo1*wo1 * wc*wc;

            A2 = 1.0/Q * wc;
            B2 = wo2/q * wc;
            C2 = wo2*wo2 * wc*wc;

            filters[k++] = new SOSBandPass(A1,B1,C1,D);
            filters[k++] = new SOSBandPass(A2,B2,C2,D);
    }
}

float BandPass::work(float sig)
{
    for (int n = 0; n < order; ++n)
    {
        sig = filters[n]->work(sig);
    }
    return sig;
}

void BandPass::reset()
{
    for (int n = 0; n < order; ++n)
    {
        filters[n]->reset();
    }
}

class BandPassBlock : public Block
{
public:
    BandPassBlock(Block * block, BandPass * filter):
        block(block),
        filter(filter)
    {
        reset();
        iter = block->get_iterator();
        value = filter->work(**iter);
        ptr = &value;
    }

    void free() {
        printf("freeing BandPassBlock...\n");
        block->free();
        delete this;
    }

    bool is_free() {
        return block->is_free();
    }

    size_t get_size() {
        return block->get_size();
    }

    void reset() {
        filter->reset();
        block->reset();
    }

    bool next() {
        if (!block->next()) {
            return false;
        }
        else {
            //value = **iter;
            value = filter->work(**iter);
            if (fabs(value) > 5) {
                printf("filter->work(%.3f): %.3f\n", **iter, value);
            }
            return true;
        }
    }

    float ** get_iterator() {
        return &ptr;
    }

    void print() {
        printf("Bandpass Block\n");
        block->print();
    }

public:
    Block * block;
    BandPass * filter;
    float value;
    float * ptr;
    float ** iter;
};

Block * BandPass::process(Block * block) {
    return new BandPassBlock(block, this);
}
