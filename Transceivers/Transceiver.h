#ifndef __TRANSCEIVER_H__
#define __TRANSCEIVER_H__

#include "../Memory/Memory.h"
#include "../RadioMsg/RadioMsg.h"
#include "../switches.h"
#include "../SignaledThread/SignaledThread.h"
#include "../Module/Module.h"
#include "../main.h"
#include "../PlotController/PlotController.h"

typedef void (*TransceiverNotify)(void * obj, RadioMsg * msg);

void transceiver_callback(void * arg, RadioMsg * msg);

class Transceiver : public SignaledThread<RadioMsg>
{
public:
    Transceiver(TransceiverNotify notify_cb, void * obj, 
            double fs=44.1E3, 
            double ftx=19E3, 
            double frx=19E3, 
            double fif=5E3, 
            double bw=3E3,
            int cycles_per_bit=40,
            PlotController * controller=NULL);

    virtual void process(RadioMsg msg);

    virtual void start(bool block);

    virtual void stop();

    virtual ~Transceiver();

    void debug(RadioMsg msg);

protected:
    TransceiverNotify notify_cb;
    void * obj;
    double fs;
    double ftx;
    double frx;
    double fif;
    double bw;
    int cycles_per_bit;
    int order;
    int spectrum_size;
    PlotController * controller;
    size_t prefix_len;
    bool * prefix;
    uint16_t * crc_table;

    /* Holds Modules */
    Module ** modules;

    /* Memory */
    Memory * tx_memory;
    Memory * rx_memory;
};
#endif
