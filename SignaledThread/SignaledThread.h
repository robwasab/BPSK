#ifndef __SIGNALED_THREAD_H__
#define __SIGNALED_THREAD_H__
#include <stdlib.h>
#include "../Log/Log.h"
#include "../Queue/Queue.h"

typedef void (*ProcessCallback_t)(void * arg);

typedef struct
{
    ProcessCallback_t process;
    void * arg;
} Task;

class SignaledThread
{
public:
    SignaledThread(size_t max_notifications=128):
    notifications(max_notifications)
    {
    }
    
    virtual ~SignaledThread() 
    {
    }

    /* wakeup the thread */
    virtual void signal() = 0;
    
    /* add data to the queue, and wakeup the thread for processing */ 
    virtual void notify(ProcessCallback_t cb, void * arg) = 0;

    /* start the thread */
    virtual void start(bool block) = 0;
    
    /* stop the thread synchronously */
    virtual void stop() = 0;
    
    /* Member Variables */
    Queue<Task> notifications;
};
#endif
