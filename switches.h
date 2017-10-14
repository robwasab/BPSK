#ifndef __SWITCH_H__

/* Prefix log prints with file location */
//#define LOG_FILE

/* Enable color prints */
//#define ENABLE_COLORS

/* Enable local loopback of signal data. If not defined, will transmit over the air. */
//#define SIMULATE

/* Enable data to be encoded in QPSK form. If not defined, will use BPSK instead */
#define QPSK_ENCODE

/* Enable QT plots. They add a lot of processing overhead, so they are enabled for development purposes only */
#define GUI

/* TERMINAL_APP means that there is a main function */
//#define TERMINAL_APP
/* MACOSX_APP means that the main function is removed, and code is expected to be called from ViewController etc. */
#define MACOSX_APP
//#define IPHONE_APP

/* Enable Constellation Plots */
#ifdef GUI
    #define DEBUG_CONSTELLATION
#endif

/* Enable Debuging of Decoder */
#ifdef GUI
    #define DEBUG_DECODER
#endif

/* Print CPU load in log */
//#define PRINT_CPU_LOAD

/* Print CPU load in QT plot */
#ifdef GUI
	#define PLOT_CPU_LOAD
#endif

/* Use FIR Filters only */
#define USE_FIR

#endif
