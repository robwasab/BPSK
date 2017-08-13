#include "TransceiverBPSK.h"
#include "../main.h"
//#include "../Constellation/Constellation.h"
#include "../TaskScheduler/TaskScheduler.h"
#include "../EncoderBPSK/StdinSource.h"
#include "../EncoderBPSK/Pulseshape.h"
#include "../EncoderBPSK/Prefix.h"
#include "../EncoderBPSK/BPSK.h"
#include "../CostasLoop/CostasLoop.h"
#include "../CostasLoop/Plottable_CostasLoop.h"
//#include "../CostasLoop/QPSK.h"
#include "../DecoderBPSK/BPSKDecoder.h"
#include "../DecoderBPSK/Plottable_BPSKDecoder.h"
#include "../Modulator/Modulator.h"
#include "../PlotSink/PlotSink.h"
#include "../SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "../Filter/BandPass.h"
#include "../Filter/FirLowPass.h"
#include "../Filter/FirHighPass.h"
#include "../Filter/FirBandPass.h"
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

void TransceiverBPSK::send(const uint8_t data[], uint8_t len)
{
    /* This is kind of ugly */
    if (modules[0] != NULL)
    {
        ByteInputInterface * tx_data = (ByteInputInterface *) modules[0];
        tx_data->process_msg(data, len);
    }
}

TransceiverBPSK::TransceiverBPSK(TransceiverNotify notify_cb, void * obj, 
        double fs, 
        double ftx, 
        double frx,
        double fif,
        double bw,
        int cycles_per_bit,
        PlotController * controller):
    Transceiver(notify_cb, obj, fs, ftx, frx, fif, bw, cycles_per_bit, controller)
{
    double fm;
    /* Transmitter Variables */
    ByteInputInterface * tx_data;
    Prefix      * tx_pref;
    BPSK        * tx_enco;
    //BandPass    * tx_bpif;
    FirBandPass * tx_bpif;
    Modulator   * tx_mdrf;
    //BandPass    * tx_bprf;
    FirBandPass * tx_bprf;

    /* Channel Variables */
    Channel * rf_chan;

    /* Receiver Variables */
    #ifdef QT_ENABLE
    PlotSink    * rx_view;
    #endif
    //BandPass    * rx_bprf;
    FirBandPass * rx_bprf;
    Modulator   * rx_modu;
    //BandPass    * rx_bpif;
    FirBandPass * rx_bpif;
    Autogain    * rx_gain;

    #ifdef DEBUG_CONSTELLATION
    Plottable_CostasLoop * rx_cost;
    #else
    CostasLoop  * rx_cost;
    #endif

    #ifdef DEBUG_DECODER
    Plottable_BPSKDecoder * rx_deco;
    #else
    BPSKDecoder * rx_deco;
    #endif

    SuppressPrint * rx_end;

    /* Transmitter Section */
    //tx_data = new StdinSource(tx_memory, transceiver_callback, this, crc_table);
    tx_data = new ByteInputInterface(tx_memory, transceiver_callback, this, crc_table);
    tx_pref = new Prefix     (tx_memory, transceiver_callback, this, prefix, prefix_len);
    tx_enco = new BPSK       (tx_memory, transceiver_callback, this, fs, fif, cycles_per_bit, 50);

    fm = ftx - fif;
    tx_bpif = new FirBandPass(tx_memory, transceiver_callback, this, fs, fif, bw, 7); 
    tx_mdrf = new Modulator  (tx_memory, transceiver_callback, this, fs, fm);
    tx_bprf = new FirBandPass(tx_memory, transceiver_callback, this, fs, ftx, bw, 7); 
    //tx_bpif = new BandPass (tx_memory, transceiver_callback, this, fs, fif, bw, order);
    //tx_bprf = new BandPass (tx_memory, transceiver_callback, this, fs, ftx, bw, order);

    /* Channel + Transducer Section */
    rf_chan = new Channel  (rx_memory, transceiver_callback, this);

    /* Receiver Section */
    #ifdef QT_ENABLE
    rx_view = new PlotSink   (rx_memory, transceiver_callback, this);
    #endif

    fm = frx - fif;
    rx_bprf = new FirBandPass(rx_memory, transceiver_callback, this, fs, frx, bw, 7);
    rx_modu = new Modulator  (rx_memory, transceiver_callback, this, fs, fm);
    rx_bpif = new FirBandPass(rx_memory, transceiver_callback, this, fs, fif, bw, 7);
    rx_gain = new Autogain   (rx_memory, transceiver_callback, this, fs);
    //rx_bprf = new BandPass (rx_memory, transceiver_callback, this, fs, frx, bw, order);
    //rx_bpif = new BandPass (rx_memory, transceiver_callback, this, fs, fif, bw, order);

    #ifdef DEBUG_CONSTELLATION
    rx_cost = new Plottable_CostasLoop(rx_memory, transceiver_callback, this, fs, fif);
    #else
    rx_cost = new CostasLoop (rx_memory, transceiver_callback, this, fs, fif);
    #endif

    #ifdef DEBUG_DECODER
    rx_deco = new Plottable_BPSKDecoder(rx_memory, transceiver_callback, this, fs, fif, prefix, prefix_len, cycles_per_bit, 0.75, crc_table,
            //PLOTTABLE_BPSK_DECODER_RESET_SIGNAL);
           PLOTTABLE_BPSK_DECODER_HIGH_PASS_SIGNAL);
    #else
    rx_deco = new BPSKDecoder(rx_memory, transceiver_callback, this, fs, fif, prefix, prefix_len, cycles_per_bit, 0.75, crc_table);
    #endif

    Module * chain[] =
    {
        tx_data, //STANDARD INPUT
        tx_pref, //PREFIX
        tx_enco, //ENCODER
        tx_bpif, //IF BANDPASS FILTER
        tx_mdrf, //RF MODULATOR
        tx_bprf, //RF BANDPASS FILTER
        rf_chan, //RF CHANNEL
        #ifdef QT_ENABLE
        rx_view, //VIEW THE WAVEFORM
        #endif
        rx_bprf, //RF BANDPASS FILTER
        rx_modu, //RF MODULATOR
        rx_bpif, //IF BANDPASS FILTER
        rx_gain, //AUTO GAIN
        rx_cost, //COSTAS LOOP
        rx_deco, //BPSK DECODER
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
    DataSource * views[] =
    {
        rx_view,
        rx_cost,
        rx_deco,
        NULL,
    };

    for (k = 0; views[k] != NULL; k++)
    {
        sources[k] = views[k];
        controller->add_plot(views[k]);
    }
    #endif
}

