#ifndef __TRANSCEIVER_BPSK_H__
#define __TRANSCEIVER_BPSK_H__

#include "Transceiver.h"

class TransceiverBPSK : public Transceiver
{
public:
    TransceiverBPSK(TransceiverNotify notify_cb, void * obj,
            double fs,
            double ftx,
            double frx,
            double fif,
            double bw,
            int cycles_per_bit,
            PlotController * controller);

    void send(const uint8_t data[], uint8_t len);

private:
};
#endif
