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

//#undef QT_ENABLE

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
    double bw = 2E3;
    int order = 4;
    int cycles_per_bit = 20;
    size_t prefix_len;
    bool * prefix;
    int spectrum_size = 1 << 10;

    generate_ml_sequence(&prefix_len, &prefix);

    prefix[0] = true;

    SuppressPrint end;

    /*

    PlotSink rx_if_rese(&end);

    BPSKDecoder rx_if_deco(&memory, &rx_if_rese, fs, fif, prefix, prefix_len, cycles_per_bit, false);

    PlotSink    rx_if_sink(&rx_if_deco);

    CostasLoop  rx_if_cost(&memory, &rx_if_sink, fs, fif, IN_PHASE_SIGNAL);

    //Spectrum    rx_if_spec(&memory, &rx_if_cost, fs, spectrum_size);

    PlotSink    rx_if_scop(&rx_if_cost);

    //Spectrum    rx_if_spec(&memory, &rx_if_scop, fs, spectrum_size);

    Autogain    rx_if_auto(&memory, &rx_if_scop, fs);

    BandPass    rx_if_band(&memory, &rx_if_auto, fs, fif, bw, order);

    Modulator   rx_rf_modu(&memory, &rx_if_band, fs, fc - fif);

    BandPass    rx_rf_band(&memory, &rx_rf_modu, fs, fc, bw, order);

    // Over the air

    //PlotSink    scope(&rx_rf_band);
    //WavSink     rx_if_wave(&memory, &rx_rf_band);

    //Spectrum    tx_rf_spec(&memory, &rx_rf_band, fs, spectrum_size);

    BandPass    tx_rf_band(&memory, &rx_rf_band, fs, fc, bw, order);

    Modulator   tx_rf_modu(&memory, &tx_rf_band, fs, fc - fif);

    BandPass    tx_if_band(&memory, &tx_rf_modu, fs, fif, bw, order);

    BPSK        tx_if_bpsk(&memory, &tx_if_band, fs, fif, cycles_per_bit, 200);

    Prefix      tx_if_pref(&memory, &tx_if_bpsk, prefix, prefix_len);

    StdinSource tx_if_sour(&memory, &tx_if_pref, &scheduler);

    */

    /* Port Audio Migration */

#ifdef QT_ENABLE
    PlotSink          scope(&end);
    BPSKDecoder rx_if_deco(&rx_memory, &scope, fs, fif, prefix, prefix_len, cycles_per_bit, false);
#else
    BPSKDecoder rx_if_deco(&rx_memory, &end, fs, fif, prefix, prefix_len, cycles_per_bit, false);
#endif

    CostasLoop  rx_if_cost(&rx_memory, &rx_if_deco, fs, fif, IN_PHASE_SIGNAL);
    Autogain    rx_if_auto(&rx_memory, &rx_if_cost, fs);
    BandPass    rx_if_band(&rx_memory, &rx_if_auto, fs, fif, bw, order);
    Modulator   rx_rf_modu(&rx_memory, &rx_if_band, fs, fc - fif);
    BandPass    rx_rf_band(&rx_memory, &rx_rf_modu, fs, fc, bw, order);
    Attenuator  rx_rf_atte(&rx_memory, &rx_rf_band, 0.01);

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

    PortAudioSimulator simulator(&rx_scheduler, &rx_memory, tx_modules, &rx_rf_atte);

    PortAudioStdin pa_stdin(&tx_memory, &simulator);

    //scheduler.start();

#ifdef QT_ENABLE
    PlotController controller(argc, argv);
    //controller.add_plot(&rx_if_sink);
    //controller.add_plot(&rx_if_scop);
    //controller.add_plot(&rx_if_rese);
    //controller.add_plot(&rx_if_spec);
    //controller.add_plot(&scope);

    controller.add_plot(&scope);
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
