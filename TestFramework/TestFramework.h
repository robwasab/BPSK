#ifndef __TEST_FRAMEWORK_H__
#define __TEST_FRAMEWORK_H__

#include "../Queue/Queue.h"
#include "../Stack/Stack.h"
#include "../Transceivers/Transceiver.h"
#include "../SignaledThread/SignaledThread.h"
#include "../Notify/Notify.h"
#include "../switches.h"

#ifdef QT_ENABLE
#include "../PlotController/PlotController.h"
#endif

#define MAX_TRANSCEIVERS 10

typedef enum 
{
    EVENT_START,
    EVENT_DONE,
    EVENT_KILL,
    EVENT_RECEIVE_DATA,
} TestEventType;

struct TestEvent 
{
    TestEventType type;
    void * data;
    size_t len;
};

typedef struct TestEvent TestEvent;

typedef void (*StateMachine)(TestEvent t);

extern void print_msg(const uint8_t msg[], uint8_t size);

class TestFramework : public SignaledThread<TestEvent>
{
public:
    TestFramework(StateMachine sm);
    ~TestFramework();

    /* Application functions */
    void smStart(StateMachine sm, TestEvent te);
    void smReturn(TestEvent te);
    void send(int id, uint8_t msg[], uint8_t len);
    int newTransceiver(double ftx, double frx, double fif, double bw, int cycles_per_bit);
    void stopTransceiver(int handle);

    /* to be only used in main... */
    void main_loop();

    /* to be only used in main... */
    void stop();

    /* to be only used in main... */
    void start(bool block);

    uint8_t receive_data[256];
    uint8_t receive_data_len;
    uint8_t receive_data_count;
protected:
    void process(TestEvent t);
private:
    Stack<StateMachine> sm_stack;
    PlotController * controller;
    Transceiver * transceivers [MAX_TRANSCEIVERS];
    int transceiver_count;
};

#endif
