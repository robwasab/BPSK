#include <math.h>
#include "QPSK.h"
#include "Biquad_LowPass.h"
#include "Integrator.h"
#include "RC_LowPass.h"
#include "QPSKBlock.h"
#define PI M_PI

static const char __name__[] = "QPSK";

const char * QPSK::name() {
    return __name__;
}

QPSK::QPSK(Memory * memory,
        Module * next,
        double fs,
        double fc,
        double biqu_fcut,
        double loop_fnat):
    CostasLoop(memory, next, fs, fc, IN_PHASE_SIGNAL, biqu_fcut, loop_fnat, 0.1),
    vco_offset_2(M_PI/4.0),
    vco_offset_3(M_PI/2.0),
    vco_offset_4(M_PI*3.0/4.0)
{
    double biqu_qual = 1.0/sqrt(2.0);
    lp2 = new Biquad_LowPass(biqu_qual, biqu_fcut, fs);
    lp4 = new Biquad_LowPass(biqu_qual, biqu_fcut, fs);
}

Block * QPSK::process(Block * block) {
    return new QPSKBlock(block, this, IN_PHASE_SIGNAL);
}

void QPSK::error_detector(float input,
        double * error,
        double * in_phase,
        double * qu_phase,
        double * lock) {

    double vco_phase;
    double vco_1, vco_2, vco_3, vco_4;
    double o1, o2, o3, o4;

    // Phase Generator
    // vco_phase is a constantly increasing phase value
    vco_phase = this->vco->value();

    // QPSK Oscillators
    vco_1 = 2.0 * cos(vco_phase);
    vco_2 = 2.0 * cos(vco_phase + vco_offset_2);
    vco_3 = 2.0 * cos(vco_phase + vco_offset_3);
    vco_4 = 2.0 * cos(vco_phase + vco_offset_4);

    // Error Generator
    o1 = this->ilp->work(input*vco_1);
    o2 = this->lp2->work(input*vco_2);
    o3 = this->qlp->work(input*vco_3);
    o4 = this->lp4->work(input*vco_4);

    *in_phase = o1;
    *qu_phase = o3;

    *error = o1 * o2 * o3 * o4;

    // Lock Detector
    *lock = this->lock_detector->work(o1 * o3, o2 * o4);
}
