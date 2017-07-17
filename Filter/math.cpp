#include <math.h>
#include "math.h"

/*
def lpf(num, fp, fs):
	ns = np.arange(num) - num/2 
	F = fp / fs
	def sinc(x):
		return np.sin(x) / x if np.abs(x) > 1E-6 else 1.0
	return np.array([ 2.0 * F * sinc(2.0 * np.pi * F * n) for n in ns])
*/

double sinc(double x)
{
    if (fabs(x) > 1E-6)
    {
        return (sin(x) / x);
    }
    else
    {
        return 1.0;
    }
}

double * lpf(int num, double fp, double fs)
{
    int k;
    double * fir;
    double F;
    double n;

    fir = new double[num];
    F = fp / fs;

    for(k = 0; k < num; k++)
    {
        n = ((double)k) - num/2.0;
        fir[k] = 2.0 * F * sinc(2.0 * M_PI * F * n);
    }

    return fir;
}

/*
def hpf(num, fp, fs):
	ns = np.arange(num) - num/2
	# find the zero index
	zero_index = -1
	for k in range(num):
		if np.abs(ns[k]) < 1E-6:
			zero_index = k
			break
	if zero_index < 0:
		raise Exception('Could not find the zero index!')

	fir = lpf(num, fp, fs)
	fir *= -1;
	fir[zero_index] += 1.0
	return fir
*/

double * hpf(int num, double fp, double fs)
{
    int k;
    int zero_index;
    double * fir;

    fir = lpf(num, fp, fs);

    for (k = 0; k < num; k++)
    {
        fir[k] *= -1;
    }

    if (num % 2 == 0 )
    {
        zero_index = num / 2;
        fir[zero_index] += 1.0;
    }
    return fir;
}

/*
def bpf(num, fp, fc, fs):
	fir = lpf(num, fp, fs)
	fir *= np.cos(2.0 * np.pi * fc * np.arange(num) / fs) * 2.0
	return fir
*/

double * bpf(int num, double fp, double fc, double fs)
{
    int k;
    double * fir;

    fir = lpf(num, fp, fs);

    for (k = 0; k < num; k++)
    {
        fir[k] *= 2.0 * cos(2.0 * M_PI * fc * k / fs);
    }

    return fir;
}

/*
def blackman(num):
	ns = np.arange(num)
	return 0.42 - 0.5 * np.cos(2.0 * np.pi * ns / (num - 1)) + 0.08 * np.cos(4.0 * np.pi * ns / (num - 1))
*/

void blackman(int num, double * fir)
{
    int k;
    double max;

    max = (double) (num - 1);

    for (k = 0; k < num; k++)
    {
        fir[k] *= 0.42 - 0.5 * cos(2.0 * M_PI * k / max) + 0.08 * cos(4.0 * M_PI * k / max);
    }
}

/*
def hamming(num):
	ns = np.arange(num)
	return 0.54 - 0.46 * np.cos(2.0 * np.pi * ns / (num - 1))
*/

void hamming(int num, double * fir)
{
    int k;
    double max;

    max = (double) (num - 1);

    for (k = 0; k < num; k++)
    {
        fir[k] *= 0.54 - 0.46 * cos(2.0 * M_PI * k / max);
    }
}

double * filter(int num, 
        double fp, 
        double fc, 
        double fs, 
        FilterType f_type, 
        WindowType w_type)
{
    double * fir;

    switch (f_type)
    {
        case LOWPASS:
            fir = lpf(num, fp, fs);
            break;

        case HIGHPASS:
            fir = hpf(num, fp, fs);
            break;

        case BANDPASS:
            fir = bpf(num, fp, fc, fs);
            break;
    }

    switch (w_type)
    {
        case BLACKMAN:
            blackman(num, fir);
            break;

        case HAMMING:
            hamming(num, fir);
            break;

        case RECTANGULAR:
            break;
    }
    return fir;
}
