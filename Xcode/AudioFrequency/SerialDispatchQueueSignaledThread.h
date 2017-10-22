//
//  SerialDispatchQueueSignaledThread.h
//  AudioFrequency
//
//  Created by Robby Tong on 10/21/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef SerialDispatchQueueSignaledThread_h
#define SerialDispatchQueueSignaledThread_h

#include "SignaledThread/SignaledThread.h"
#include <Dispatch/Dispatch.h>

class SerialDispatchQueueSignaledThread : public SignaledThread
{
public:
    SerialDispatchQueueSignaledThread(size_t max_notifications=128);
    
    ~SerialDispatchQueueSignaledThread();
    
    void signal();
    void notify(ProcessCallback_t cb, void * arg);
    void start(bool block);
    void stop();
private:
    dispatch_queue_t mQueue;
    
};

#endif /* SerialDispatchQueueSignaledThread_h */
