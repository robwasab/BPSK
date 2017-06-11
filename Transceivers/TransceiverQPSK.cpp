#include "TransceiverQPSK.h"
#include "../main.h"
#include "../Constellation/Constellation.h"
#include "../TaskScheduler/TaskScheduler.h"
#include "../EncoderBPSK/StdinSource.h"
#include "../EncoderBPSK/Pulseshape.h"
#include "../EncoderBPSK/Prefix.h"
#include "../EncoderBPSK/BPSK.h"
#include "../EncoderQPSK/QPSK_StdinSource.h"
#include "../EncoderQPSK/QPSK_Encode.h"
#include "../EncoderQPSK/QPSK_Prefix.h"
#include "../CostasLoop/CostasLoop.h"
#include "../CostasLoop/QPSK.h"
#include "../DecoderBPSK/BPSKDecoder.h"
#include "../Modulator/Modulator.h"
#include "../PlotSink/PlotSink.h"
#include "../SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "../Filter/BandPass.h"
#include "../WavSink/WavSink.h"
#include "../Memory/Memory.h"
#include "../Autogain/Autogain.h"
#include "../PortAudio/PortAudioSimulator.h"
#include "../PortAudio/PortAudioStdin.h"
#include "../Attenuator/Attenuator.h"
#include "../SuppressPrint/SuppressPrint.h"
#include "../Channel/Channel.h"
#include "../switches.h"

#ifdef QT_ENABLE
#include "../PlotController/PlotController.h"
#endif

TransceiverQPSK::TransceiverQPSK(TransceiverNotify notify_cb, void * obj, double fs, double fc):
    Transceiver(notify_cb, obj, fs, fc)
{
    /* Transmitter variables */
    BandPass * tx_bpif;
    Modulator * tx_mdrf;
    BandPass * tx_bprf;

    #ifdef QPSK_ENCODE
    QPSK_StdinSource * tx_data;
    QPSK_Prefix * tx_pref;
    QPSK_Encode * tx_enco;
    #else
    StdinSource * tx_data;
    Prefix * tx_pref;
    BPSK   * tx_enco;
    #endif

    /* Channel Variables */
    Channel * rf_chan;

    /* Receiver Variables */
    #ifdef QT_ENABLE
    Constellation    * rx_plot;
    PlotSink         * rx_view;
    SpectrumAnalyzer * rx_spec;
    #endif
    SuppressPrint * rx_end;
    QPSK      * rx_deco;
    Autogain  * rx_gain;
    BandPass  * rx_bpif;
    Modulator * rx_modu;
    BandPass  * rx_bprf;

    /* Transmitter Section */
    double fm = fc - fif;
    tx_mdrf = new Modulator(tx_memory, transceiver_callback, this, fs, fm);
    tx_bpif = new BandPass (tx_memory, transceiver_callback, this, fs, fif, bw, order);
    tx_bprf = new BandPass (tx_memory, transceiver_callback, this, fs, fc, bw, order);

    #ifdef QPSK_ENCODE
    tx_data = new QPSK_StdinSource(tx_memory, transceiver_callback, this);
    tx_pref = new QPSK_Prefix (tx_memory, transceiver_callback, this);
    tx_enco = new QPSK_Encode (tx_memory, transceiver_callback, this, fs, fif, cycles_per_bit, 50);
    #else
    tx_data = new StdinSource(tx_memory, transceiver_callback, this);
    tx_pref = new Prefix (tx_memory, transceiver_callback, this, prefix, prefix_len);
    tx_enco = new BPSK   (tx_memory, transceiver_callback, this, fs, fif, cycles_per_bit, 50);
    #endif

    /* Channel + Transducer Section */
    rf_chan = new Channel(rx_memory, transceiver_callback, this);

    /* Receiver Section */
    rx_end = new SuppressPrint;

    #ifdef QT_ENABLE
    rx_plot = new Constellation   (rx_memory, transceiver_callback, this, fs, 1024);
    rx_spec = new SpectrumAnalyzer(rx_memory, transceiver_callback, this, fs, 256);
    rx_view = new PlotSink        (rx_memory, transceiver_callback, this);
    #endif

    rx_deco = new QPSK     (rx_memory, transceiver_callback, this, fs, fif);
    rx_gain = new Autogain (rx_memory, transceiver_callback, this, fs);
    rx_bpif = new BandPass (rx_memory, transceiver_callback, this, fs, fif, bw, order);
    rx_modu = new Modulator(rx_memory, transceiver_callback, this, fs, fm);
    rx_bprf = new BandPass (rx_memory, transceiver_callback, this, fs, fc , bw, order);

    Module * chain[] =
    {
        tx_data, //STANDARD INPUT
        tx_pref, //PREFIX
        tx_enco, //ENCODER
        tx_bpif, //IF BANDPASS FILTER
        tx_mdrf, //RF MODULATOR
        tx_bprf, //RF BANDPASS FILTER
        rf_chan, //RF CHANNEL
        rx_view, //VIEW THE WAVEFORM
        rx_bprf, //RF BANDPASS FILTER
        rx_modu, //RF MODULATOR
        rx_bpif, //IF BANDPASS FILTER
        rx_gain, //AUTO GAIN
        rx_deco, //PLL
        #ifdef QT_ENABLE
        rx_plot, //CONSTELLATION PLOT
        //rx_spec, //SPECTRUM ANALYZER
        #endif
        rx_end,  //END
        NULL,
    };

    RadioMsg msg(CMD_RESET_ALL);

    int k;
    for (k = 0; chain[k] != NULL; k++)
    {
        chain[k]->id = k;
        modules[k] = chain[k];
        modules[k]->dispatch(&msg);
    }

    #ifdef QT_ENABLE
    controller->add_plot(rx_view);
    controller->add_plot(rx_plot);
    //controller->add_plot(rx_spec);
    #endif
}

