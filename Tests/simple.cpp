#include "simple.h"
#include "../TestFramework/TestFramework.h"
#include "../Log/Log.h"

static double ftx = 18E3;
static double frx = 18E3;
static double fif = 3E3;
static double bw = 1E3;
static int cycles_per_bit = 10;

void simple(TestEvent e)
{
    static TestFramework * test_frame;
    static int handle_1;

    TestEvent te;

    switch (e.type)
    {
        case EVENT_START:
            test_frame = (TestFramework *) e.data;
            handle_1 = test_frame->newTransceiver(ftx, frx, fif, bw, cycles_per_bit);
            break;

        case EVENT_KILL:
            LOG("EVENT_KILL\n");
            LOG("killing transceiver %d\n", handle_1);
            test_frame->stopTransceiver(handle_1);
            test_frame->smReturn(e);
            break;

        case EVENT_RECEIVE_DATA:
            LOG("Got data!\n");
            break;

        default:
            break;
    }
}
