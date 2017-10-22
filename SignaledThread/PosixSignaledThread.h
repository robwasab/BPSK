//
//  PosixSignaledThread.h
//  AudioFrequency
//
//  Created by Robby Tong on 10/21/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef PosixSignaledThread_h
#define PosixSignaledThread_h
#include <stdlib.h>
#include <pthread.h>
#include "SignaledThread.h"
#include "../Log/Log.h"
#include "../Queue/Queue.h"

class PosixSignaledThread: public SignaledThread
{
public:
    PosixSignaledThread(size_t max_notifications=128);
    ~PosixSignaledThread();
    
    /* add data to the queue, and wakeup the thread for processing */
    void notify(ProcessCallback_t cb, void * arg);
    
    /* start the thread */
    void start(bool block);
    
    /* stop the thread */
    void stop();
    
    /* should quit? */
    bool should_quit();
    
    /* wait until signal */
    void wait();
    
    void signal();
    
    /* Member Variables */
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t main;
    
    bool quit;
    void lock();
    void unlock();
};

#endif /* PosixSignaledThread_h */
