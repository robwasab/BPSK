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

typedef enum {
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
    TestFramework(StateMachine sm, double ftx, double frx, double fif, double bw, int cycles_per_bit);
    ~TestFramework();
    void smStart(StateMachine sm, TestEvent te);
    void smReturn(TestEvent te);
    void main_loop();
    void stop();
    void start(bool block);
    uint8_t receive_data[256];
    uint8_t receive_data_len;
    uint8_t receive_data_count;
protected:
    void process(TestEvent t);
private:
    Stack<StateMachine> sm_stack;
    Transceiver * transceiver;
    PlotController * controller;
};

#endif
