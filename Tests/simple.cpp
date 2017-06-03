#include "simple.h"

void simple(TestEvent e)
{
    switch (e.type)
    {
        case TestEvent::EVENT_START:
            LOG("STARTING SIMPLE TEST!\n");
            break;
        default:
            break;
    }
}
