#include "TestFramework.h"
#include "../Transceivers/TransceiverBPSK.h"
#include <assert.h>
#include <string.h>

#ifdef GUI
#include "../PlotController/QTPlotController.h"
#endif

#define MAX_STATE_MACHINE 128

char test_event_strings[][64] = 
{
    [EVENT_START] = "EVENT_START",
    [EVENT_DONE]  = "EVENT_DONE",
    [EVENT_KILL]  = "EVENT_KILL",
    [EVENT_RECEIVE_DATA] = "EVENT_RECEIVE_DATA",
    [EVENT_USER_INPUT] = "EVENT_USER_INPUT",
};

static
bool isChar(char c, const char check[])
{
    int k;
    for (k = 0; check[k] != '\0'; k++)
    {
        if (c == check[k])
        {
            return true;
        }
    }
    return false;
}

void print_msg(const uint8_t msg[], uint8_t size)
{
    char c;
    MAGENTA;
    for (int k = 0; k < size; k++)
    {
        c = (char) msg[k];

        if (c == '\n') 
        {
            printf("\\n");
        }
        else if ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z'))
        {
            printf("%c", c);
        }
        else if (isChar(c, "1234567890 !@#$%^&*()_-+={}[]|:;<>,.?/""'"))
        {
            printf("%c", c);
        }
        else if (c == '\0')
        {
            printf("█");
        }
        else
        {
            printf("?");
        }
    }
    printf("\n");
    ENDC;
}

void TestFramework_cb(void * obj, RadioMsg * msg)
{
    TestFramework * self = (TestFramework *) obj;
    TestEvent te;

    switch (msg->type)
    {
        case NOTIFY_USER_REQUEST_QUIT:
            LOG("User has requested to quit\n");
            te.type = EVENT_KILL;
            self->notify(te);
            break;
            
        case NOTIFY_DATA_START:
            LOG("Incomming data: %hhu\n", msg->args[0]);
            self->receive_data_count = 0;
            self->receive_data_len = msg->args[0];
            break;

        case NOTIFY_DATA_BODY:
            LOG("Received data body!\n");
            if ((self->receive_data_len - self->receive_data_count) > RADIO_ARG_SIZE)
            {
                memcpy(&self->receive_data[self->receive_data_count], msg->args, RADIO_ARG_SIZE);
                self->receive_data_count += RADIO_ARG_SIZE;
            }
            else
            {
                memcpy(&self->receive_data[self->receive_data_count], msg->args, self->receive_data_len - self->receive_data_count);
                self->receive_data_count += self->receive_data_len - self->receive_data_count;

                LOG("Assembled message: ");
                print_msg(self->receive_data, self->receive_data_len);

                uint8_t * frwd_data = (uint8_t *) malloc(self->receive_data_len);
                te.type = EVENT_RECEIVE_DATA;
                te.data = frwd_data;
                te.len = self->receive_data_len;
                self->notify(te);
            }
            break;

        default:
            break;
    }
}

void plotcontroller_close_callback(void * arg)
{
    TestEvent te;
    TestFramework * self;

    LOG("PlotController window closed, shutting down...\n");
    
    self = (TestFramework *) arg;

    te.type = EVENT_KILL;
    te.data = NULL;
    te.len = 0;
    self->notify(te);
}


TestFramework::TestFramework(StateMachine sm):
    SignaledThread(128),
    sm_stack(MAX_STATE_MACHINE),
    controller(NULL),
    transceiver_count(0)
{
    memset(transceivers, 0, sizeof(transceivers));
    memset(receive_data, 0, sizeof(receive_data));
    receive_data_len = 0;
    receive_data_count = 0;

    #ifdef GUI
    controller = new QTPlotController(_argc, _argv);
    controller->set_close_cb(plotcontroller_close_callback, this);
    #else
    controller = NULL;
    #endif

    TestEvent te = {EVENT_START, this, 0};
    smStart(sm, te);
}

int TestFramework::newTransceiver(double ftx, double frx, double fif, double bw, int cycles_per_bit)
{
    if (transceiver_count >= MAX_TRANSCEIVERS)
    {
        return -1;
    }

    transceivers[transceiver_count] = 
    new TransceiverBPSK(TestFramework_cb, this, 44.1E3, ftx, frx, fif, bw, cycles_per_bit, controller);

    transceivers[transceiver_count]->start(false);

    return transceiver_count++;
}

void TestFramework::stopTransceiver(int handle)
{
    if (handle < 0 || handle >= transceiver_count)
    {
        return;
    }
    if (transceivers[handle] != NULL)
    {
        LOG("Killing transceiver #%d...\n", handle);
        LOG("Begining top down thread killing...\n");
        transceivers[handle]->stop();
        //delete transceivers[handle];
        //transceivers[handle] = NULL;
    }
}

void TestFramework::send(int handle, const uint8_t msg[], uint8_t len)
{
    if (handle < 0 || handle >= transceiver_count)
    {
        return;
    }
    transceivers[handle]->send(msg, len);
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
    if (controller != NULL)
    {
        delete (QTPlotController *) controller;
    }

    for (int k = 0; k < MAX_TRANSCEIVERS; k++)
    {
        if (transceivers[k] != NULL)
        {
            delete transceivers[k];
        }
    }
}

/* Don't call manualy, this is called in main_loop */
void TestFramework::start(bool block)
{
    #ifdef GUI
    SignaledThread::start(false);

    /* start the plot controller */
    if (controller != NULL)
    {
        (void) controller->run();
    }
    #else 
    SignaledThread::start(block);
    #endif
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
            case EVENT_KILL:
                lock();
                quit = true;
                unlock();
                break;

            case EVENT_START:
            case EVENT_DONE:
            case EVENT_RECEIVE_DATA:
            case EVENT_USER_INPUT:
                LOG("No state machine to handle event!\n");
                break;
        }
    }

    if (t.data != NULL && t.len > 0)
    {
        free(t.data);
    }
}

