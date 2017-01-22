#include "SpectrumAnalyzer/SpectrumAnalyzer.h"
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
#include "Filter/BandPass.h"
#include "WavSink/WavSink.h"
#include "Memory/Memory.h"
#include "Autogain/Autogain.h"
#include "PortAudio/PortAudioSimulator.h"
#include "PortAudio/PortAudioStdin.h"
#include "Attenuator/Attenuator.h"

#ifdef QT_ENABLE
#include "PlotController/PlotController.h"
#endif

#undef QT_ENABLE

typedef SpectrumAnalyzer Spectrum;

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

TaskScheduler rx_scheduler(128);
Memory tx_memory;
Memory rx_memory;

int main(int argc, char ** argv)
{
    double fs = 44.1E3;
    double fc = 18E3;
    double fif = 3E3;
    double bw = 3E3;
    int order = 6;
    int cycles_per_bit = 10;
    size_t prefix_len;
    bool * prefix;
    int spectrum_size = 1 << 10;

    generate_ml_sequence(&prefix_len, &prefix);

    prefix[0] = true;

    SuppressPrint end;

#ifdef QT_ENABLE
    PlotSink          scope(&end);
    BPSKDecoder rx_if_deco(&rx_memory, &scope, fs, fif, prefix, prefix_len, cycles_per_bit, false);
    //PlotSink    freq(&rx_if_deco);
    CostasLoop  rx_if_cost(&rx_memory, &rx_if_deco, fs, fif, IN_PHASE_SIGNAL);
    //PlotSink    auto_scope(&rx_if_cost);
    Autogain    rx_if_auto(&rx_memory, &rx_if_cost, fs);
#else
    BPSKDecoder rx_if_deco(&rx_memory, &end, fs, fif, prefix, prefix_len, cycles_per_bit, false);
    CostasLoop  rx_if_cost(&rx_memory, &rx_if_deco, fs, fif, IN_PHASE_SIGNAL);
    Autogain    rx_if_auto(&rx_memory, &rx_if_cost, fs);
#endif

    BandPass  rx_if_band(&rx_memory, &rx_if_auto, fs, fif, bw, order);
    Modulator rx_rf_modu(&rx_memory, &rx_if_band, fs, fc - fif);
    BandPass  rx_rf_band(&rx_memory, &rx_rf_modu, fs, fc, bw, order);

    BandPass  bprf(&tx_memory, NULL, fs, fc, bw, order);
    Modulator mdrf(&tx_memory, NULL, fs, fc - fif);
    BandPass  bpif(&tx_memory, NULL, fs, fif, bw, order);
    BPSK      bpsk(&tx_memory, NULL, fs, fif, cycles_per_bit, 250);
    Prefix    pref(&tx_memory, NULL, prefix, prefix_len);

    Module * tx_modules[16] = {NULL};
    tx_modules[0] = &pref;
    tx_modules[1] = &bpsk;
    tx_modules[2] = &bpif;
    tx_modules[3] = &mdrf;
    tx_modules[4] = &bprf;
    tx_modules[5] = NULL;
    /*
    tx_modules[5] = &rx_rf_band;
    tx_modules[6] = &rx_rf_modu;
    tx_modules[7] = &rx_if_band;
    tx_modules[8] = &rx_if_auto;
    tx_modules[9] = &rx_if_cost;
    tx_modules[10] = NULL;
    */

    PortAudioSimulator simulator(&rx_scheduler, &rx_memory, tx_modules, &rx_rf_band);

    PortAudioStdin pa_stdin(&tx_memory, &simulator);

    //scheduler.start();

#ifdef QT_ENABLE
    PlotController controller(argc, argv);
    controller.add_plot(&scope);
    //controller.add_plot(&auto_scope);
    //controller.add_plot(&freq);
    rx_scheduler.start();
    simulator.start();
    pa_stdin.start(false);
    int ret = controller.run();
    rx_scheduler.stop();
    simulator.stop();
    return ret;
#else 
    rx_scheduler.start();
    simulator.start();
    pa_stdin.start(true);
    rx_scheduler.stop();
    simulator.stop();
    return 0;
#endif
}
