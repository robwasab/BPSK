#include <math.h>
#include "CostasLoop.h"
#include "Biquad_LowPass.h"
#include "Integrator.h"
#include "RC_LowPass.h"
#include "CostasLoopBlock.h"
#define PI M_PI

char __name__[] = "CostasLoop";

const char * CostasLoop::name() {
    return __name__;
}

CostasLoop::CostasLoop(Memory * memory, 
        Module * next,
        double fs,
        double fc,
        SignalType type):
    Module(memory, next),
    fs(fs),
    fc(fc),
    type(type)
{
    // Quality factor of in and out of phase low pass filters
    double biqu_qual = 1.0/sqrt(2.0);

    // Cutoff frequency for the in and out of phase lpf [Hz]
    // Determined by separate program which I will also include
    // In CostasLoop directory - it was written in Python
    double biqu_fcut = 600.0;

    // Damping ratio of this control system
    double loop_zeta = 1.0/sqrt(2.0);

    // Natural Frequency of this control system [Hz]
    // Which was also determined by aforementioned program
    double loop_fnat = 200.0;

    // The variable G and a so that the loop achieves the above responses
    G = 4.0*M_PI*loop_zeta*loop_fnat;
    a = loop_fnat*M_PI/loop_zeta;
    
    ilp = new Biquad_LowPass(biqu_qual, biqu_fcut, this->fs);
    qlp = new Biquad_LowPass(biqu_qual, biqu_fcut, this->fs);

    inc = 2.0*PI*this->fc;
    
    vco = new Integrator(fs);
    amp = new Integrator(fs);

    // Variables to help generate measure frequency
    last_vco_phase = 0.0;
    freq_filter = new Biquad_LowPass(0.707106, 100.0, this->fs);
    HZ_PER_RAD = this->fs/2.0/PI;

    // Lock Detector
    lock_detector = new LockDetector(this->fs);
    lock_rc = new RC_LowPass(0.01, this->fs);
    reset();
}

CostasLoop::~CostasLoop()
{
    delete ilp;
    delete qlp;
    delete vco;
    delete amp;
    delete freq_filter;
    delete lock_detector;
    delete lock_rc;
}

void CostasLoop::reset() {
    last_vco_phase = 0.0;
    ilp->reset();
    qlp->reset();
    vco->reset();
    amp->reset();
    freq_filter->reset();
    lock_detector->reset();
    lock_rc->reset();
}

float CostasLoop::work(float input, 
        float * freq_ptr, 
        float * lock_ptr, 
        float * error) 
{
    double vco_phase;
    double cos_vco;
    double sin_vco;
    double in_phase;
    double qu_phase;
    double lock;
    double phase_der;

    // Phase Generator
    // vco_phase is a constantly increasing phase value
    vco_phase = this->vco->value();

    // Oscillator
    cos_vco = cos(vco_phase);
    sin_vco =-sin(vco_phase);

    // Error Generator
    in_phase = 2.0*this->ilp->work(input*cos_vco);
    qu_phase = 2.0*this->qlp->work(input*sin_vco);

    // Update Loop Integrators
    double s2 = in_phase*qu_phase;
    if (error) {
        *error = s2;
    }
    double s3 = this->G * s2;
    double s4 = this->a * s3;
    double s5 = this->amp->work(s4);
    double s6 = s3 + s5;
    this->vco->work(this->inc + s6);

    lock = this->lock_detector->work(in_phase, qu_phase);
    if (lock_ptr) {
        *lock_ptr = this->lock_rc->work(lock);
    }

    phase_der = (vco_phase - this->last_vco_phase) * this->HZ_PER_RAD;
    if (freq_ptr) {
        *freq_ptr = this->freq_filter->work(phase_der);
    }

    this->last_vco_phase = vco_phase;
    //return 0.7*in_phase_signal;
    return in_phase;
}

/*
Block * CostasLoop::process(Block * block) {
    static char errors[][25] = {
        {"No error"},
        {"Allocation error"} };
    int error = 0;
    float ** out_iter = NULL;
    float ** block_iter = NULL;

    reset();

    Block * out = memory->allocate(block->get_size());

    if (!out) {
        error = 1;
        goto fail;
    }

    out_iter = out->get_iterator();
    block_iter = block->get_iterator();
    block->reset();

    do
    {
        **out_iter = work(**block_iter, NULL, NULL, NULL); 

    } while(out->next() && block->next());

    block->free();

    return out;

fail:
    RED;
    printf("%s\n", errors[error]);
    ENDC;
    block->free();
    if (out) {
        out->free();
    }
    return NULL;
}
*/

Block * CostasLoop::process(Block * block) {

    return new CostasLoopBlock(block, this, type);
}
