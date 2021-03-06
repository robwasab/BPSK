#ifndef __TRANSCEIVER_PSK8_H__
#define __TRANSCEIVER_PSK8_H__

#include "Transceiver.h"

class TransceiverPSK8 : public Transceiver
{
public:
    TransceiverPSK8(TransceiverNotify notify_cb, void * obj, 
            double fs, 
            double ftx, 
            double frx, 
            double fif,
            double bw, 
            int cycles_per_bit);

private:
};
#endif
