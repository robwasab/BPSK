#include "simple.h"
#include "../TestFramework/TestFramework.h"
#include "../Log/Log.h"

#define TE TestEvent

void simple(TestEvent e)
{
    static TestFramework * test_frame;
    TestEvent te;

    switch (e.type)
    {
        case EVENT_START:
            test_frame = (TestFramework *) e.data;
            break;

        case EVENT_KILL:
            LOG("EVENT_KILL\n");
            test_frame->smReturn(e);
            break;

        case EVENT_RECEIVE_DATA:
            LOG("Got data!\n");
            break;

        default:
            break;
    }
}
