#ifndef __HIGH_PASS_H__
#define __HIGH_PASS_H__

class HighPass
{
public:
    HighPass(float tau, float fs)
    {
        k = tau * fs / (tau * fs + 1);
        in_last = 0.0;
        out_last = 0.0;
    }

    float work(float in) 
    {
        float out;
        out = k * (in - in_last + out_last);
        out_last = out;
        in_last = in;
        return out;
    }

    double value() 
    {
        return out_last;
    }

    void reset() 
    {
        in_last = 0.0;
        out_last = 0.0;
    }

private:
    float k;
    float in_last;
    float out_last;
};

#endif

