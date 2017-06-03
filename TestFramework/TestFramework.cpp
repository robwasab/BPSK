#include "TestFramework.h"
#include "../Transceivers/TransceiverQPSK.h"
#include <assert.h>

#define MAX_STATE_MACHINE 128

TestFramework::TestFramework(TransceiverType type, StateMachine sm):
    SignaledThread(),
    sm_stack(MAX_STATE_MACHINE)
{
    switch(type)
    {
        case PSK2:
            assert(false);
            break;

        case PSK4:
            transceiver = new TransceiverQPSK(44.1E3, 18E3);
            break;

        case PSK8:
            assert(false);
            break;

        case PSK16:
            break;
    }
    SignaledThread::start(false);

    TestEvent te = {TestEvent::EVENT_START, (uint8_t *) &transceiver, sizeof(Transceiver *)};
    smStart(sm, te);
}

void TestFramework::smStart(StateMachine sm, TestEvent te)
{
    LOG("STARTING STATE MACHINE!\n");
    sm_stack.push(sm);
    notify(te);
}

TestFramework::~TestFramework()
{
}

void TestFramework::start(bool block)
{
    LOG("STARTING TRANSCEIVER!\n");
    transceiver->start();
}

void TestFramework::process(TestEvent t)
{
    StateMachine * sm = sm_stack.peek();

    if (sm != NULL)
    {
        (*sm)(t);
    }
    else
    {
        LOG("State Machine is NULL!\n");
    }
}
