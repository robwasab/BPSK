#ifndef __RC_LOWPASS_H__
#define __RC_LOWPASS_H__

class RC_LowPass
{
public:
    RC_LowPass(double tau, double fs) {
        tau_fs = tau*fs;
        last = 0.0;
    }

    double work(double sig) {
        last = (sig + tau_fs * last)/(1.0 + tau_fs);
        return last;
    }

    double value() {
        return last;
    }

    void reset() {
        last = 0.0;
    }

    double tau_fs;
    double last;
};

#endif
