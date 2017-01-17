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
        long double out = input + lasty;
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

class PeriodicIntegrator : public Integrator
{
public:
    PeriodicIntegrator(double fs, double max):
        Integrator(fs)
    {
        this->max = max;
    }

    double value() {
        return Integrator::value();
    }

    double work(double input) {
        long double out = input + lasty;
        lasty = input + out;
        return out/twofs;
    }

    void reset() {
        lasty = 0;
    }
private:
    double max;
    long double lasty;
    double twofs;
};

#endif
