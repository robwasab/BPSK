#ifndef __PSK8_ENCODE_H__
#define __PSK8_ENCODE_H__

#include "../Module/Module.h"

class PSK8_SigGen : public Module
{
public:
    PSK8_SigGen(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            float fs, 
            float fc, 
            int cycles_per_sym,
            int training_cycles = 0);

    Block * process(Block * symbols);
    const char * name();
    ~PSK8_SigGen() { }
    void dispatch(RadioMsg * msg);
private:
    double fs;
    double fc;
    int cycles_per_sym;
    int training_cycles;
    double phase;
    double psk8_phase;
    double psk8_ampli;
};
#endif
