#include "simple.h"
#include "../TestFramework/TestFramework.h"
#include "../Log/Log.h"

void simple(TestEvent e)
{
    static TestFramework * test_frame;
    TestEvent te;

    switch (e.type)
    {
        case TestEvent::EVENT_START:
            test_frame = (TestFramework *) e.data;
            break;

        case TestEvent::EVENT_KILL:
            LOG("EVENT_KILL\n");
            test_frame->smReturn(e);
            break;

        default:
            break;
    }
}
