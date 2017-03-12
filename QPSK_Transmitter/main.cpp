#include "../TaskScheduler/TaskScheduler.h"
#include "../Transmitter/BPSK.h"
#include "../Memory/Memory.H"
#include "QPSK_StdinSource.h"
#include "QPSK_Prefix.h"
#include "QPSK_Encode.h"

int main()
{
    Memory memory;
    TaskScheduler scheduler(128);
    double fs = 44.1E3;
    double fif = 3E3;
    int cycles_per_bit = 10;

    QPSK_Encode      qpsk(&memory, NULL, fs, fif, cycles_per_bit, 250);
    QPSK_Prefix      prefix(&memory, &qpsk);
    QPSK_StdinSource source(&memory, &prefix, &scheduler);

    scheduler.start();
    source.start(true);

    return 0;
}
