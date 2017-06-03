#include "SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "Constellation/Constellation.h"
#include "TaskScheduler/TaskScheduler.h"
#include "MaximumLength/generator.h"
#include "EncoderBPSK/StdinSource.h"
#include "EncoderBPSK/Pulseshape.h"
#include "EncoderBPSK/Prefix.h"
#include "EncoderBPSK/BPSK.h"
#include "EncoderQPSK/QPSK_Encode.h"
#include "EncoderQPSK/QPSK_Prefix.h"
#include "CostasLoop/CostasLoop.h"
#include "CostasLoop/QPSK.h"
#include "DecoderBPSK/BPSKDecoder.h"
#include "Modulator/Modulator.h"
#include "PlotSink/PlotSink.h"
#include "Filter/BandPass.h"
#include "WavSink/WavSink.h"
#include "Memory/Memory.h"
#include "Autogain/Autogain.h"
#include "PortAudio/PortAudioSimulator.h"
#include "PortAudio/PortAudioStdin.h"
#include "Attenuator/Attenuator.h"
#include "SuppressPrint/SuppressPrint.h"
#include "switches.h"
#include "Transceivers/Transceiver.h"

#ifdef QT_ENABLE
#include "PlotController/PlotController.h"
#endif

typedef SpectrumAnalyzer Spectrum;

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
    int cycles_per_bit = 5;
    size_t prefix_len;
    bool * prefix;
    int spectrum_size = 1 << 10;
    Module * tx_modules[16] = {NULL};

    generate_ml_sequence(&prefix_len, &prefix);

    SuppressPrint end;

#ifdef QT_ENABLE
    //PlotSink      scope(&end);
    //BPSKDecoder rx_if_deco(&rx_memory, &scope, fs, fif, prefix, prefix_len, cycles_per_bit, false);
    Constellation constellation(&rx_memory, &end, fs, 1024);
    #ifdef QPSK_ENCODE
    QPSK        rx_if_deco(&rx_memory, &constellation, fs, fif); 
    #else
    CostasLoop  rx_if_deco(&rx_memory, &constellation, fs, fif, IN_PHASE_SIGNAL);
    #endif
    Autogain    rx_if_auto(&rx_memory, &rx_if_deco, fs);
    #else
    BPSKDecoder rx_if_deco(&rx_memory, &end, fs, fif, prefix, prefix_len, cycles_per_bit, false);
    QPSK        rx_if_qpsk(&rx_memory, &rx_if_deco, fs, fif); 
    CostasLoop  rx_if_cost(&rx_memory, &rx_if_deco, fs, fif, IN_PHASE_SIGNAL);
    Autogain    rx_if_auto(&rx_memory, &rx_if_qpsk, fs);
#endif
    BandPass  rx_if_band(&rx_memory, &rx_if_auto, fs, fif, bw, order);
    Modulator rx_rf_modu(&rx_memory, &rx_if_band, fs, fc - fif);
    BandPass  rx_rf_band(&rx_memory, &rx_rf_modu, fs, fc, bw, order);

/* Transmitter Definition */

/* Filtering Section */
    BandPass    bpif(&tx_memory, NULL, fs, fif, bw, order);
    Modulator   mdrf(&tx_memory, NULL, fs, fc - fif);
    BandPass    bprf(&tx_memory, NULL, fs, fc, bw, 8);

#ifdef QPSK_ENCODE
    QPSK_Prefix pref(&tx_memory, NULL);
    QPSK_Encode enco(&tx_memory, NULL, fs, fif, cycles_per_bit, 50);
#else
    Prefix pref(&tx_memory, NULL, prefix, prefix_len);
    BPSK   enco(&tx_memory, NULL, fs, fif, cycles_per_bit, 50);
#endif

    tx_modules[0] = &pref;
    tx_modules[1] = &enco;
    tx_modules[2] = &bpif;
    tx_modules[3] = &mdrf;
    tx_modules[4] = &bprf;
    tx_modules[5] =  NULL;

    PortAudioSimulator simulator(&rx_scheduler, &rx_memory, tx_modules, &rx_rf_band);
    PortAudioStdin pa_stdin(&tx_memory, &simulator);

#ifdef QT_ENABLE
    PlotController controller(argc, argv);
    //controller.add_plot(&auto_scope);
    //controller.add_plot(&freq);
    controller.add_plot(&constellation);
    //controller.add_plot(&scope);
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
