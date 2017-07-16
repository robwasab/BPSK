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
    void stop();
    Block * process(Block * block);
    void dispatch(RadioMsg * msg);
    const char * name() {
        return _STDIN_SOURCE_NAME_; 
    }
    int fd[2];
private:
    pthread_t main;
};

#endif
