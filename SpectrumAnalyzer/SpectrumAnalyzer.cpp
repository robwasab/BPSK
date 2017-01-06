#include "SpectrumAnalyzer.h"
#include <string.h>

const char __SPECTRUM_ANALYZER_NAME__[] = "SpectrumAnalyzer";

const char * SpectrumAnalyzer::name() {
    return __SPECTRUM_ANALYZER_NAME__;
}

SpectrumAnalyzer::SpectrumAnalyzer(int n, double fs):
    n(n)
{
    freqs = new float[n];
    data = new float[n];

    for (int k = 0; k < n; ++k) 
    {
        freqs[k] = fs * ((float) k) / ((float) n);
    }

    memset(data, 0, sizeof(float) * n);
}

size_t SpectrumAnalyzer::size() {
    return n;
}

Point SpectrumAnalyzer::get_data(size_t index) {
    Point p;
    p.x = freqs[index];
    p.y = data[index];
    return p;
}

void SpectrumAnalyzer::next() {
}

Point SpectrumAnalyzer::get_origin() {
    Point p;
    p.x = 0.0;
    p.y = 0.0;
}

Point SpectrumAnalyzer::get_lengths() {
    Point p;
    p.x = n;
    p.y = 1.0;
}

bool SpectrumAnalyzer::valid() {
    return false;
}
