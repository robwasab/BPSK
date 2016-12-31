#ifndef __STDIN_SOURCE_H__
#define __STDIN_SOURCE_H__

#include <pthread.h>
#include "../Module/Module.h"
#include "../TaskScheduler/TaskScheduler.h"

const char _STDIN_SOURCE_NAME_[] = "StdinSource";

class StdinSource: public Module
{
public:
    StdinSource(Memory * memory, Module * next, TaskScheduler * scheduler);
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
