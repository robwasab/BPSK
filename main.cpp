#include "TaskScheduler/TaskScheduler.h"
#include "MaximumLength/generator.h"
#include "Transmitter/StdinSource.h"
#include "Transmitter/Pulseshape.h"
#include "Transmitter/Prefix.h"
#include "Transmitter/BPSK.h"
#include "CostasLoop/CostasLoop.h"
#include "Receiver/BPSKDecoder.h"
#include "PlotSink/PlotSink.h"
#include "Filter/Bandpass.h"
#include "WavSink/WavSink.h"
#include "Memory/Memory.h"

#ifdef QT_ENABLE
#include "PlotController/PlotController.h"
#endif

TaskScheduler scheduler(128);
Memory memory;

int main(int argc, char ** argv)
{
    double fs = 44.1E3;
    double fc = 2E3;
    double bw = 1E3;
    int order = 2;
    int cycles_per_bit = 20;
    size_t prefix_len;
    bool * prefix;

    generate_ml_sequence(&prefix_len, &prefix);
    prefix[0] = true;


    BPSKDecoder deco(&memory, NULL, fs, fc, prefix, prefix_len, cycles_per_bit, false);

    PlotSink sink(&deco);
    CostasLoop  cost(&memory, &deco, fs, fc, IN_PHASE_SIGNAL);
    //WavSink     wave(&memory, &cost);
    BandPass    band(&memory, &cost, fs, fc, bw, order);
    BPSK        bpsk(&memory, &band, fs, fc, cycles_per_bit, 100);
    Prefix      pref(&memory, &bpsk, prefix, prefix_len);
    StdinSource sour(&memory, &pref, &scheduler);
    scheduler.start();

#ifdef QT_ENABLE
    PlotController controller(argc, argv);
    sour.start(false);
    controller.add_plot(&pulseshape);
    controller.add_plot(&sink);
    return controller.run();
#else 
    sour.start(true);
    return 0;
#endif
}
