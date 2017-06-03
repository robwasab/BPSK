#ifndef __TRANSCEIVER_H__
#define __TRANSCEIVER_H__

#include "../TaskScheduler/TaskScheduler.h"
#include "../MaximumLength/generator.h"
#include "../Memory/Memory.h"
#include "../PortAudio/PortAudioSimulator.h"
#include "../PortAudio/PortAudioStdin.h"
#include "../switches.h"
#include "../main.h"

#ifdef QT_ENABLE
#include "../PlotController/PlotController.h"
#endif

#define MAX_MODULES 32

struct RadioMsg
{

};

class Transceiver
{
public:
    Transceiver(double fs=44.1E3, double fc=18E3, double fif=3E3, double bw=3E3):
        fs(fs),
        fc(fc),
        fif(fif),
        bw(bw),
        order(6),
        cycles_per_bit(5),
        spectrum_size(1 << 10)
    {
        generate_ml_sequence(&prefix_len, &prefix);

        /* Initialize Objects */
        tx_memory = new Memory();
        rx_memory = new Memory();
        tx_modules = new Module*[MAX_MODULES];
        rx_modules = new Module*[MAX_MODULES];

        int k;
        for (k = 0; k < MAX_MODULES; k++)
        {
            tx_modules[k] = NULL;
            rx_modules[k] = NULL;
        }
        rx_scheduler =  new TaskScheduler(128);

    #ifdef QT_ENABLE
        controller = new PlotController(_argc, _argv);
    #endif
    }

    /* Start the thread */
    int start()
    {
        portaudio = new PortAudioSimulator(rx_scheduler, rx_memory, tx_modules, rx_modules[0]);
        pa_stdin  = new PortAudioStdin(tx_memory, portaudio);

#ifdef QT_ENABLE
        /* start the receiver first */
        rx_scheduler->start();

        /* start portaudio */
        portaudio->start();

        /* start read from stdin loop, do not block, controller will block instead */
        pa_stdin->start(false);

        /* start the plot controller */
        int ret = controller->run();

        /* controller has stopped, stop receiving */
        rx_scheduler->stop();

        /* stop port audio */
        portaudio->stop();

        return ret;
#else 
        /* start the receiver first */
        rx_scheduler->start();

        /* start portaudio */
        simulator->start();

        /* start read from stdin loop, block */
        pa_stdin->start(true);

        /* stdin loop has stopped, stop receiving */
        rx_scheduler->stop();

        /* stop portaudio */
        portaudio->stop();
        return 0;
#endif
    }

    virtual ~Transceiver()
    {
        int k;
        for (k = 0; k < MAX_MODULES; k++)
        {
            if (tx_modules[k] != NULL)
            {
                LOG("Deleting Transmitter Module #%d...\n", k);
                delete tx_modules[k];
            }
            if (rx_modules[k] != NULL)
            {
                LOG("Deleting Receiver Module #%d...\n", k);
                delete rx_modules[k];
            }
        }
        LOG("delete [] tx_modules...\n");
        delete [] tx_modules;

        LOG("delete [] rx_modules...\n");
        delete [] rx_modules;

        LOG("delete pa_stdin...\n");
        delete pa_stdin;

        LOG("delete portaudio...\n");
        delete portaudio;

        LOG("delete rx_scheduler...\n");
        delete rx_scheduler;

        LOG("delete tx_memory...\n");
        delete tx_memory;

        LOG("delete rx_modules...\n");
        delete rx_memory;

        #ifdef QT_ENABLE
        LOG("delete controller...\n");
        delete controller;
        #endif
    }

protected:
    double fs;
    double fc;
    double fif;
    double bw;
    int order;
    int cycles_per_bit;
    int spectrum_size;
    size_t prefix_len;
    bool * prefix;

    /* Port Audio Objects */
    PortAudioStdin * pa_stdin;
    PortAudioSimulator * portaudio;

    /* Receiver Scheduler */
    TaskScheduler * rx_scheduler;

    /* Holds Transmitter Objects */
    Module ** tx_modules;

    /* Holds Receiver Objects */
    Module ** rx_modules;

    /* Memory */
    Memory * tx_memory;
    Memory * rx_memory;

#ifdef QT_ENABLE
    PlotController * controller;
#endif

};
#endif
