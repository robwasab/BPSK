#include "TransceiverQPSK.h"
#include "../main.h"
#include "../Constellation/Constellation.h"
#include "../TaskScheduler/TaskScheduler.h"
#include "../EncoderBPSK/StdinSource.h"
#include "../EncoderBPSK/Pulseshape.h"
#include "../EncoderBPSK/Prefix.h"
#include "../EncoderBPSK/BPSK.h"
#include "../EncoderQPSK/QPSK_Encode.h"
#include "../EncoderQPSK/QPSK_Prefix.h"
#include "../CostasLoop/CostasLoop.h"
#include "../CostasLoop/QPSK.h"
#include "../DecoderBPSK/BPSKDecoder.h"
#include "../Modulator/Modulator.h"
#include "../PlotSink/PlotSink.h"
#include "../Filter/BandPass.h"
#include "../WavSink/WavSink.h"
#include "../Memory/Memory.h"
#include "../Autogain/Autogain.h"
#include "../PortAudio/PortAudioSimulator.h"
#include "../PortAudio/PortAudioStdin.h"
#include "../Attenuator/Attenuator.h"
#include "../SuppressPrint/SuppressPrint.h"
#include "../switches.h"

#ifdef QT_ENABLE
#include "../PlotController/PlotController.h"
#endif

void TransceiverQPSK::init_receiver()
{
    #ifdef QT_ENABLE
    Constellation * constellation;
    #endif
    SuppressPrint * end;
    QPSK      * deco;
    Autogain  * gain;
    BandPass  * bpif;
    Modulator * modu;
    BandPass  * bprf;

    end = new SuppressPrint;

    #ifdef QT_ENABLE
    constellation = new Constellation(rx_memory, end, fs, 1024);
    controller->add_plot(constellation);
    #endif

    deco = new QPSK     (rx_memory, constellation, fs, fif);
    gain = new Autogain (rx_memory, deco, fs);
    bpif = new BandPass (rx_memory, gain, fs, fif, bw, order);
    modu = new Modulator(rx_memory, bpif, fs, fc - fif);
    bprf = new BandPass (rx_memory, modu, fs, fc , bw, order);

    rx_modules[0] = bprf;
    rx_modules[1] = modu;
    rx_modules[2] = bpif;
    rx_modules[3] = gain;
    rx_modules[4] = deco;
    #ifdef QT_ENABLE
    rx_modules[5] = constellation;
    rx_modules[6] = end;
    rx_modules[7] = NULL;
    #else
    rx_modules[5] = end;
    rx_modules[6] = NULL;
    #endif
}

void TransceiverQPSK::init_transmitter()
{
    BandPass  * bpif = new BandPass (tx_memory, NULL, fs, fif, bw, order);
    Modulator * mdrf = new Modulator(tx_memory, NULL, fs, fc - fif);
    BandPass  * bprf = new BandPass (tx_memory, NULL, fs, fc, bw, 8);

#ifdef QPSK_ENCODE
    QPSK_Prefix * pref = new QPSK_Prefix (tx_memory, NULL);
    QPSK_Encode * enco = new QPSK_Encode (tx_memory, NULL, fs, fif, cycles_per_bit, 50);
#else
    Prefix * pref = new Prefix (tx_memory, NULL, prefix, prefix_len);
    BPSK   * enco = new BPSK   (tx_memory, NULL, fs, fif, cycles_per_bit, 50);
#endif

    tx_modules[0] = pref;
    tx_modules[1] = enco;
    tx_modules[2] = bpif;
    tx_modules[3] = mdrf;
    tx_modules[4] = bprf;
    tx_modules[5] = NULL;
}

TransceiverQPSK::TransceiverQPSK(double fs, double fc):
    Transceiver(fs, fc)
{
    init_transmitter();
    init_receiver();
}

int TransceiverQPSK::start()
{
    return Transceiver::start();
}

