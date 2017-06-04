#ifndef __STDIN_SOURCE_H__
#define __STDIN_SOURCE_H__

#include <pthread.h>
#include "../Module/Module.h"

const char _STDIN_SOURCE_NAME_[] = "StdinSource";

class StdinSource: public Module
{
public:
    StdinSource(Memory * memory, TransceiverCallback cb, void * trans);
    void start(bool block = true);
    Block * process(Block * block);
    using Module::name;
    const char * name() {
        return _STDIN_SOURCE_NAME_; 
    }
private:
    pthread_t main;
};

#endif
