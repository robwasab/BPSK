#ifndef __QPSK_STDIN_SOURCE_H__
#define __QPSK_STDIN_SOURCE_H__

#include <pthread.h>
#include "../Module/Module.h"
#include "../TaskScheduler/TaskScheduler.h"


class QPSK_StdinSource: public Module
{
public:
    QPSK_StdinSource(Memory * memory, TransceiverCallback cb, void * trans);
    void start(bool block = true);
    void stop();
    Block * process(Block * block);
    void dispatch(RadioMsg * msg);
    const char * name();
private:
    pthread_t main;
};

#endif
