#include "TaskScheduler/TaskScheduler.h"
#include "MaximumLength/generator.h"
#include "Transmitter/StdinSource.h"
#include "Transmitter/Pulseshape.h"
#include "Transmitter/Prefix.h"
#include "Transmitter/BPSK.h"
#include "CostasLoop/CostasLoop.h"
#include "Receiver/BPSKDecoder.h"
#include "Modulator/Modulator.h"
#include "PlotSink/PlotSink.h"
#include "Filter/Bandpass.h"
#include "WavSink/WavSink.h"
#include "Memory/Memory.h"

#ifdef QT_ENABLE
#include "PlotController/PlotController.h"
#endif

static char __name__[] = "SuppressPrint";

class SuppressPrint : public Module
{
public:
    SuppressPrint():
    Module(NULL, NULL) {}

    const char * name() {
        return __name__;
    }

    Block * process(Block * in) {
        in->free();
        return NULL;
    }
};

TaskScheduler scheduler(128);
Memory memory;

int main(int argc, char ** argv)
{
    double fs = 44.1E3;
    double fc = 17E3;
    double fif = 2E3;
    double bw = 1E3;
    int order = 2;
    int cycles_per_bit = 20;
    size_t prefix_len;
    bool * prefix;

    generate_ml_sequence(&prefix_len, &prefix);
    prefix[0] = true;

    SuppressPrint end;
    PlotSink rx_if_rese(&end);
    BPSKDecoder rx_if_deco(&memory, &rx_if_rese, fs, fif, prefix, prefix_len, cycles_per_bit, false);
    PlotSink    rx_if_sink(&rx_if_deco);
    CostasLoop  rx_if_cost(&memory, &rx_if_sink, fs, fif, IN_PHASE_SIGNAL);
    BandPass    rx_if_band(&memory, &rx_if_cost, fs, fif, bw, order);
    Modulator   rx_rf_modu(&memory, &rx_if_band, fs, fc - fif);
    BandPass    rx_rf_band(&memory, &rx_rf_modu, fs, fc, bw, order);
    /* Over the air */
    //WavSink     wave(&memory, &cost);
    //PlotSink    scope(&rx_rf_band);
    BandPass    tx_rf_band(&memory, &rx_rf_band, fs, fc, bw, order);
    Modulator   tx_rf_modu(&memory, &tx_rf_band, fs, fc - fif);
    BandPass    tx_if_band(&memory, &tx_rf_modu, fs, fif, bw, order);
    BPSK        tx_if_bpsk(&memory, &tx_if_band, fs, fif, cycles_per_bit, 200);
    Prefix      tx_if_pref(&memory, &tx_if_bpsk, prefix, prefix_len);
    StdinSource tx_if_sour(&memory, &tx_if_pref, &scheduler);
    scheduler.start();

#ifdef QT_ENABLE
    PlotController controller(argc, argv);
    tx_if_sour.start(false);
    controller.add_plot(&rx_if_sink);
    controller.add_plot(&rx_if_rese);
    //controller.add_plot(&scope);
    return controller.run();
#else 
    tx_if_sour.start(true);
    return 0;
#endif
}
