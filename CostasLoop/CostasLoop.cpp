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
        TransceiverCallback cb,
        void * trans,
        double fs,
        double fc,
        SignalType type, 
        double biqu_fcut,
        double loop_fnat,
        double lock_thresh):
    SecondOrderPLL(memory, cb, trans, fs, fc, loop_fnat, 1.0/sqrt(2.0)),
    type(type)
{
    // Quality factor of in and out of phase low pass filters
    double biqu_qual = 1.0/sqrt(2.0);

    ilp = new Biquad_LowPass(biqu_qual, biqu_fcut, fs);
    qlp = new Biquad_LowPass(biqu_qual, biqu_fcut, fs);

    // Variables to help generate measure frequency
    last_vco_phase = 0.0;
    freq_filter = new Biquad_LowPass(0.707106, 100.0, fs);
    HZ_PER_RAD = fs/2.0/PI;

    // Lock Detector
    lock_detector = new LockDetector(fs, lock_thresh);
    lock_rc = new RC_LowPass(0.01, fs);
    no_lock_timer = new RC_LowPass(0.25, fs);

    sent_lock_notify = false;
    sent_lost_lock_notify = false;

    reset();
}

CostasLoop::~CostasLoop()
{
    delete ilp;
    delete qlp;
    delete freq_filter;
    delete lock_detector;
    delete lock_rc;
    delete no_lock_timer;
}

void CostasLoop::reset() {
    SecondOrderPLL::reset();
    last_vco_phase = 0.0;
    ilp->reset();
    qlp->reset();
    freq_filter->reset();
    lock_detector->reset();
    lock_rc->reset();
    no_lock_timer->reset();
}

void CostasLoop::error_detector(float input, 
        double * error, 
        double * in_phase, 
        double * qu_phase,
        double * lock)
{
    double vco_phase;
    double cos_vco;
    double sin_vco;

    // Phase Generator
    // vco_phase is a constantly increasing phase value
    vco_phase = this->vco->value();

    // Oscillator
    cos_vco = cos(vco_phase);
    sin_vco =-sin(vco_phase);

    // Error Generator
    *in_phase = 2.0*this->ilp->work(input*cos_vco);
    *qu_phase = 2.0*this->qlp->work(input*sin_vco);

    *error = (*in_phase) * (*qu_phase);

    // Lock Detector
    *lock = this->lock_detector->work(*in_phase, *qu_phase);
}

void CostasLoop::work(float input, 
        float * in_phase_ptr,
        float * qu_phase_ptr,
        float * freq_ptr, 
        float * lock_ptr, 
        float * error_ptr) 
{
    double error;
    double in_phase;
    double qu_phase;
    double lock;
    double vco_phase;
    double phase_der;

    // Update Loop Integrators
    error_detector(input, &error, &in_phase, &qu_phase, &lock);

    if (in_phase_ptr) 
    {
        *in_phase_ptr = in_phase;
    }

    if (qu_phase_ptr) 
    {
        *qu_phase_ptr = qu_phase;
    }

    double s2 = error;

    if (error_ptr) 
    {
        *error_ptr = s2;
    }

    double s3 = this->G * s2;
    double s4 = this->a * s3;
    double s5 = this->amp->work(s4);
    double s6 = s3 + s5;
    this->vco->work(this->inc + s6);

    if (lock_ptr) 
    {
        *lock_ptr = this->lock_rc->work(lock);
    }

    vco_phase = this->vco->value();

    phase_der = (vco_phase - this->last_vco_phase) * this->HZ_PER_RAD;

    if (freq_ptr) 
    {
        *freq_ptr = this->freq_filter->work(phase_der);
    }

    this->last_vco_phase = vco_phase;

    if (!lock_ptr)
    {
        return;
    }

    if (*lock_ptr < 0.8)
    {
        sent_lock_notify = false;

        if (!sent_lost_lock_notify)
        {
            RadioMsg lost_lock_notify(NOTIFY_PLL_LOST_LOCK);
            broadcast(&lost_lock_notify);
            sent_lost_lock_notify = true;
            //LOG("Broadcasting notification: %s lost lock\n", name());
        }

        no_lock_timer->work(1.0);

        if (no_lock_timer->value() > 0.99)
        {
            //LOG("Broadcasting notification: %s reset\n", name());
            reset();
            no_lock_timer->reset();

            RadioMsg reset_notify(NOTIFY_PLL_RESET);
            broadcast(&reset_notify);
        }
    }
    else
    {
        sent_lost_lock_notify = false;

        if (!sent_lock_notify)
        {
            RadioMsg lock_notify(NOTIFY_PLL_LOCK);
            broadcast(&lock_notify);
            sent_lock_notify = true;
            //LOG("Broadcasting notification: %s regained lock\n", name());
        }
    }
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

Block * CostasLoop::process(Block * block)
{
    return new CostasLoopBlock(block, this, type);
}

