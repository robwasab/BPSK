#ifndef __TRANSCEIVER_QPSK_H__
#define __TRANSCEIVER_QPSK_H__

#include "Transceiver.h"

class TransceiverQPSK : public Transceiver
{
public:
    TransceiverQPSK(double fs, double fc);

    /* Start the thread */
    int start();

private:
    void init_receiver();
    void init_transmitter();
};
#endif
