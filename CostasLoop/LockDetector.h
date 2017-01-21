#ifndef __LOCK_DETECTOR_H__
#define __LOCK_DETECTOR_H__
#include <stdint.h>
#include <stdbool.h>
#include "Biquad_LowPass.h"
#include "LockDetector.h"

class LockDetector
{
public:
    LockDetector(float fs, float thresh = 0.5) {
        in_phase_lp = new Biquad_LowPass(0.707106, 10.0, fs);
        qu_phase_lp = new Biquad_LowPass(0.707106, 10.0, fs);
        this->thresh = thresh;
    }

    ~LockDetector() {
        delete in_phase_lp;
        delete qu_phase_lp;
    }

    void reset() {
        in_phase_lp->reset();
        qu_phase_lp->reset();
    }

    bool work(float in_phase, float qu_phase) {
        in_phase = in_phase_lp->work(in_phase*in_phase);
        qu_phase = qu_phase_lp->work(qu_phase*qu_phase);
        if (in_phase > thresh && qu_phase < thresh)
            return true;
        else
            return false;
    }

private:
    Biquad_LowPass * in_phase_lp;
    Biquad_LowPass * qu_phase_lp;
    float thresh;
};

#endif
