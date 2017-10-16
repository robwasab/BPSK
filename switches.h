#ifndef __SWITCH_H__

/*******************************************************************************
 MARK: PASS AS ARGUMENTS TO COMPILER TO ENABLE
 *******************************************************************************/

// -D ENABLE_COLORS //Enables terminal colors, doesn't work in XCode
// -D LOG_FILE      //Prefixes log prints with file location

// These control the targets and control which functions are linked
// -D TERMINAL_APP  // Uses QT framework for graphs, starts from main
// -D MACOSX_APP    // Uses Cocoa framework for graphs, radio is started from Cocoa interface.
// -D IPHONE_APP

// These control the transmitter / receiver driver
// -D SIMULATE
// -D PORTAUDIO
// -D AUDIOTOOLBOX

/*******************************************************************************
 MARK: UNCOMMENT / COMMENT TO ENABLE OR DISABLE
 *******************************************************************************/

#define AUDIOTOOLBOX
/* Prefix log prints with file location */
//#define LOG_FILE

/* Enable local loopback of signal data. If not defined, will transmit over the air. */
//#define SIMULATE

/* Enable data to be encoded in QPSK form. If not defined, will use BPSK instead */
#define QPSK_ENCODE

/* Enable QT plots. They add a lot of processing overhead, so they are enabled for development purposes only */
#define GUI

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
