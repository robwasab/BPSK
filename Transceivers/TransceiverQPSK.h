#ifndef __TRANSCEIVER_QPSK_H__
#define __TRANSCEIVER_QPSK_H__

#include "Transceiver.h"

class TransceiverQPSK : public Transceiver
{
public:
    TransceiverQPSK(TransceiverNotify notify_cb, void * obj, 
            double fs, 
            double ftx, 
            double frx,
            double fif,
            double bw, 
            int cycles_per_bit);

private:
};
#endif
