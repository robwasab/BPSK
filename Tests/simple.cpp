#include "simple.h"
#include "../TestFramework/TestFramework.h"
#include "../Log/Log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static double ftx1 = 18E3;
static double frx1 = 18E3;
static double ftx2 = 8E3;
static double frx2 = 8E3;
static double fif = 4E3;
static double bw = 2E3;
static int cycles_per_bit = 10;

void simple(TestEvent e)
{
    static char prefix[] = "radio 2: ";
    static char buf[2][256] = {{0}, {0}};
    static char dummy_msg[] = "arbitrary message from the other radio!\n";
    static TestFramework * test_frame;
    static int handle_1 = 0;
    static int handle_2 = 0;

    TestEvent te;

    SM_MSG;

    switch (e.type)
    {
        case EVENT_START:
            test_frame = (TestFramework *) e.data;
            handle_1 = test_frame->newTransceiver(ftx1, frx1, fif, bw, cycles_per_bit);
            //handle_2 = test_frame->newTransceiver(ftx2, frx1, fif, bw, cycles_per_bit);
            break;

        case EVENT_KILL:
            LOG("killing transceiver %d\n", handle_1);
            test_frame->stopTransceiver(handle_1);

            /*
            LOG("killing transceiver %d\n", handle_2);
            test_frame->stopTransceiver(handle_2);
            */

            test_frame->smReturn(e);
            break;

        case EVENT_RECEIVE_DATA:
            LOG("Got data!\n");
            break;

        case EVENT_USER_INPUT:
            test_frame->send(handle_1, (uint8_t *)e.data, e.len);
            /*
            memset(buf[0], 0, sizeof(buf[0]));
            memcpy(&buf[0][0], prefix, strlen(prefix));
            memcpy(&buf[0][strlen(prefix)], e.data, e.len);
            test_frame->send(handle_2, (uint8_t *)buf[0], strlen(buf[0]));
            */
            break;

        default:
            break;
    }
}
