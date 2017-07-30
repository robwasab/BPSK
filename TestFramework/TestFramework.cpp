#include "TestFramework.h"
#include "../Transceivers/TransceiverBPSK.h"
#include "../Transceivers/TransceiverQPSK.h"
#include "../Transceivers/TransceiverPSK8.h"
#include <assert.h>

#define MAX_STATE_MACHINE 128

static
void TestFramework_cb(void * obj, RadioMsg * msg)
{
    TestFramework * self = (TestFramework *) obj;
    TestEvent te;

    switch (msg->type)
    {
        case NOTIFY_USER_REQUEST_QUIT:
            LOG("User has requested to quit\n");
            te.type = TestEvent::EVENT_KILL;
            self->notify(te);
            break;
            
        case NOTIFY_DATA_START:
            LOG("Incomming data: %hhu\n", msg->args[0]);
            break;

        case NOTIFY_DATA_BODY:
            LOG("Received data body!\n");
            break;

        default:
            break;
    }
}


TestFramework::TestFramework(TransceiverType type, StateMachine sm, double ftx, double frx, double fif, double bw, int cycles_per_bit):
    SignaledThread(128),
    sm_stack(MAX_STATE_MACHINE)
{
    switch(type)
    {
        case PSK2:
            transceiver = new TransceiverBPSK(TestFramework_cb, this, 44.1E3, 
                    ftx, 
                    frx,
                    fif,
                    bw,
                    cycles_per_bit);
            break;

        case PSK4:
            transceiver = new TransceiverQPSK(TestFramework_cb, this, 44.1E3, 
                    ftx,
                    frx,
                    fif,
                    bw,
                    cycles_per_bit);
            break;

        case PSK8:
            transceiver = new TransceiverPSK8(TestFramework_cb, this, 44.1E3,
                    ftx,
                    frx,
                    fif,
                    bw,
                    cycles_per_bit);
            break;

        case PSK16:
            break;
    }

    TestEvent te = {TestEvent::EVENT_START, this, 0};
    smStart(sm, te);
}

void TestFramework::smStart(StateMachine sm, TestEvent te)
{
    sm_stack.push(sm);
    notify(te);
}

void TestFramework::smReturn(TestEvent te)
{
    StateMachine sm;
    sm_stack.pop(&sm);
    notify(te);
}

TestFramework::~TestFramework()
{
    delete transceiver;
}

void TestFramework::start(bool block)
{
    SignaledThread::start(false);
    transceiver->start(false);
}

void TestFramework::stop()
{
    SignaledThread::stop();
}

void TestFramework::main_loop()
{
    start(false);
    pthread_join(main, NULL);
    LOG("Joined...\n");
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
        switch (t.type)
        {
            case TestEvent::EVENT_KILL:
                LOG("Beginning top down thread killing...\n");
                transceiver->stop();
                lock();
                quit = true;
                unlock();
                break;
            case TestEvent::EVENT_START:
            case TestEvent::EVENT_DONE:
                break;
        }
    }
}
