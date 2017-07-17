#ifndef __FILTER_MATH_H__
#define __FILTER_MATH_H__

enum WindowType
{
    BLACKMAN,
    HAMMING,
    RECTANGULAR
};

enum FilterType
{
    LOWPASS,
    HIGHPASS,
    BANDPASS
};

typedef FilterType FilterType;
typedef WindowType WindowType;

/* PARAMETERS:
 * num : The number filter coefficients.
 * fp  : Passband frequency. For LowPass and HighPass, it is the -6 dB frequency. 
 *       For BandPass filters, it is bw / 2.0.
 * fc  : BandPass filter only. BandPass center frequency.
 * fs  : Sampling frequency.
 * f_type : LOWPASS, HIGHPASS, or BANDPASS.
 * w_type : HAMMING, BLACKMAN, or RECTANGULAR.
 */

double * filter(int num, 
        double fp, 
        double fc, 
        double fs, 
        FilterType f_type, 
        WindowType w_type);

#endif

