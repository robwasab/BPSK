//
//  PosixSignaledThread.cpp
//  AudioFrequency
//
//  Created by Robby Tong on 10/21/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#include "PosixSignaledThread.h"

static void * task_loop(void * args);

PosixSignaledThread::PosixSignaledThread(size_t max_notifications):
    SignaledThread(max_notifications),
    quit(false)
{
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
}

PosixSignaledThread::~PosixSignaledThread()
{
    
}

void PosixSignaledThread::notify(ProcessCallback_t cb, void * arg)
{
    Task task = {.process = cb, .arg = arg};
    notifications.add(task);
    signal();
}

void PosixSignaledThread::start(bool block)
{
    pthread_create(&main, NULL, task_loop, this);
    if (block)
    {
        pthread_join(main, NULL);
    }
}

void PosixSignaledThread::stop()
{
    lock();
    quit = true;
    unlock();
    signal();
    pthread_join(main, NULL);
}

bool PosixSignaledThread::should_quit()
{
    bool should_quit;
    lock();
    should_quit = quit;
    unlock();
    return should_quit;
}

void PosixSignaledThread::wait()
{
    lock();
    pthread_cond_wait(&cond, &mutex);
    unlock();
}

static
void * task_loop(void * args)
{
    PosixSignaledThread * self = (PosixSignaledThread *) args;
    
    while(1)
    {
        if (self->notifications.size() > 0)
        {
            /* dispatch event */
            Task task;
            int rc;
            rc = self->notifications.get(&task);
            
            if (!rc && task.process != NULL)
            {
                task.process(task.arg);
            }
        }
        else if (self->should_quit())
        {
            break;
        }
        else
        {
            self->wait();
        }
    }
    return NULL;
}

void PosixSignaledThread::signal()
{
    lock();
    pthread_cond_signal(&cond);
    //pthread_cond_broadcast(&cond);
    unlock();
}

/* Operating system dependent thread functions */

void PosixSignaledThread::lock()
{
    pthread_mutex_lock(&mutex);
}

void PosixSignaledThread::unlock()
{
    pthread_mutex_unlock(&mutex);
}
