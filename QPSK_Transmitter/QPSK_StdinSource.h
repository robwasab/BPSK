#ifndef __QPSK_STDIN_SOURCE_H__
#define __QPSK_STDIN_SOURCE_H__

#include <pthread.h>
#include "../Module/Module.h"
#include "../TaskScheduler/TaskScheduler.h"

const char _STDIN_SOURCE_NAME_[] = "QPSK_StdinSource";

class QPSK_StdinSource: public Module
{
public:
    QPSK_StdinSource(Memory * memory, Module * next, TaskScheduler * scheduler);
    void start(bool block = true);
    Block * process(Block * block);
    TaskScheduler * scheduler;
    using Module::name;
    const char * name() {
        return _STDIN_SOURCE_NAME_; 
    }
private:
    pthread_t main;
};

#endif
