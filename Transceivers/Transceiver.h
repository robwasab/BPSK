#ifndef __TRANSCEIVER_H__
#define __TRANSCEIVER_H__

#include "../Memory/Memory.h"
#include "../RadioMsg/RadioMsg.h"
#include "../switches.h"
#include "../SignaledThread/SignaledThread.h"
#include "../Module/Module.h"
#include "../main.h"

#ifdef QT_ENABLE
#include "../PlotController/PlotController.h"
#endif

typedef void (*TransceiverNotify)(void * obj, RadioMsg * msg);

void transceiver_callback(void * arg, RadioMsg * msg);

class Transceiver : public SignaledThread<RadioMsg>
{
public:
    Transceiver(TransceiverNotify notify_cb, void * obj, double fs=44.1E3, double fc=19E3, double fif=4E3, double bw=3E3);

    virtual void process(RadioMsg msg);

    virtual void start(bool block);

    virtual void stop();

    virtual ~Transceiver();

    void debug(RadioMsg msg);

protected:
    TransceiverNotify notify_cb;
    void * obj;
    double fs;
    double fc;
    double fif;
    double bw;
    int order;
    int cycles_per_bit;
    int spectrum_size;
    size_t prefix_len;
    bool * prefix;

    /* Holds Modules */
    Module ** modules;

    /* Memory */
    Memory * tx_memory;
    Memory * rx_memory;

#ifdef QT_ENABLE
    PlotController * controller;
#endif

};
#endif
