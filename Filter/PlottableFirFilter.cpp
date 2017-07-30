#include "PlottableFirFilter.h"

PlottableFirFilter::PlottableFirFilter(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver,
            size_t n, 
            const double coefs[]):
    FirFilter(memory, cb, transceiver, n, coefs)
{
    int k;
    max = coefs[0];
    min = coefs[0];
    LOG("n: %zu\n", n);

    for (k = 1; k < n; k++)
    {
        if (coefs[k] > max)
        {
            max = coefs[k];
        }

        if (coefs[k] < min)
        {
            min = coefs[k];
        }
    }

    mvalid = false;
}

size_t PlottableFirFilter::size()
{
    return n;
}

Point PlottableFirFilter::get_data(size_t index)
{
    Point p;
    p.x = index;
    p.y = coefs[index];
    return p;
}

void PlottableFirFilter::next()
{

}

Point PlottableFirFilter::get_origin()
{
    Point p;
    p.x = 0.0;
    p.y = min;
    return p;
}

Point PlottableFirFilter::get_lengths()
{
    Point p;
    p.x = n;
    p.y = max - min;
    return p;
}

bool PlottableFirFilter::valid()
{
    if (mvalid == false)
    {
        mvalid = true;
        return false;
    }
    return true;
}

int PlottableFirFilter::get_updateInterval()
{
    return 1000;
}

const char * PlottableFirFilter::name()
{
    return FirFilter::name();
}

