#ifndef __TEST_FRAMEWORK_H__
#define __TEST_FRAMEWORK_H__

#include "../Queue/Queue.h"
#include "../Stack/Stack.h"
#include "../Transceivers/Transceiver.h"
#include "../SignaledThread/SignaledThread.h"
#include "../Notify/Notify.h"

struct TestEvent 
{
    enum {
        EVENT_START,
        EVENT_DONE,
    } type;
    uint8_t * data;
    size_t len;
};

typedef struct TestEvent TestEvent;

enum TransceiverType
{
    PSK2,
    PSK4,
    PSK8,
    PSK16
};

typedef enum TransceiverType TransceiverType;

typedef void (*StateMachine)(TestEvent t);

class TestFramework : public SignaledThread<TestEvent>
{
public:
    TestFramework(TransceiverType type, StateMachine sm);
    ~TestFramework();
    void smStart(StateMachine sm, TestEvent te);
    void start(bool block);
protected:
    void process(TestEvent t);
private:
    Stack<StateMachine> sm_stack;
    Transceiver * transceiver;
};

#endif
