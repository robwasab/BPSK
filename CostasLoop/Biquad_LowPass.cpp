#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "Biquad_LowPass.h"
#define PI M_PI

void Biquad_LowPass::init(double A, 
        double omega, 
        double sn, 
        double cs, 
        double alpha, 
        double beta)
{
    b0 = (1.0 - cs) / 2.0;
    b1 =  1.0 - cs;
    b2 = (1.0 - cs) / 2.0;
    a0 =  1.0 + alpha;
    a1 = -2.0 * cs;
    a2 =  1.0 - alpha;
}

float Biquad_LowPass::work(float x0)
{
    float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;
    return y0;
}

void Biquad_LowPass::reset()
{
    x2 = 0;
    x1 = 0;
    y2 = 0;
    y1 = 0;
}

Biquad_LowPass::Biquad_LowPass(double Q, double fc, double fs)
{
    double A = 1.0;
    double omega = 2.0*PI*fc/fs;
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn/(2.0*Q);
    double beta = sqrt(A+A);

    init(A, omega, sn, cs, alpha, beta);
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
}
