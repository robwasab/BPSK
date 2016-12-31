#include "PlotController/PlotController.h"
#include "TaskScheduler/TaskScheduler.h"
#include "Transmitter/StdinSource.h"
#include "Transmitter/Pulseshape.h"
#include "Transmitter/Prefix.h"
#include "Transmitter/BPSK.h"
#include "PlotSink/PlotSink.h"
#include "Filter/Bandpass.h"
#include "Memory/Memory.h"

TaskScheduler scheduler(128);
Memory memory;

int main(int argc, char ** argv)
{

    PlotController controller(argc, argv);

    PlotSink sink(NULL);

    BandPass    band(&memory, &sink, 44.1E3, 3E3, 1E3, 2);
    BPSK        bpsk(&memory, &band, 44.1E3, 3E3, 20, 20);
    Prefix      pref(&memory, &bpsk);
    StdinSource sour(&memory, &pref, &scheduler);

    scheduler.start();
    sour.start(false);

    //controller.add_plot(&pulseshape);
    controller.add_plot(&sink);
    return controller.run();
}
