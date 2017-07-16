#ifndef __SWITCH_H__

/* Enable local loopback of signal data. If not defined, will transmit over the air. */
//#define SIMULATE 

/* Enable data to be encoded in QPSK form. If not defined, will use BPSK instead */
#define QPSK_ENCODE

/* Enable QT plots. They add a lot of processing overhead, so they are enabled for development purposes only */
#define QT_ENABLE

/* Enable Constellation Plots */
#ifdef QT_ENABLE
    #define DEBUG_CONSTELLATION
#endif

#endif
