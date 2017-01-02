#ifndef __INTEGRATOR_H__
#define __INTEGRATOR_H__

class Integrator
{
public:
    Integrator(double fs) {
        lasty = 0.0;
        twofs = 2.0 * fs;
    }

    double value() {
        return (double) (lasty/twofs);
    }

    double work(double input) {
        double out = input + lasty;
        lasty = input + out;
        return out/twofs;
    }

    void reset() {
        lasty = 0;
    }

private:
    long double lasty;
    double twofs;
};

#endif
