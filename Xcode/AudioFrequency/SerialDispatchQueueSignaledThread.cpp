//
//  SerialDispatchQueueSignaledThread.cpp
//  AudioFrequency
//
//  Created by Robby Tong on 10/21/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#include <stdio.h>
#include "SerialDispatchQueueSignaledThread.h"
#include <Dispatch/Dispatch.h>
#include "Log/Log.h"

SerialDispatchQueueSignaledThread::
SerialDispatchQueueSignaledThread(size_t max_notifications):
SignaledThread(max_notifications)
{
    //mQueue = dispatch_queue_create("com.AudioFrequency.SerialDispatchQueueSignaledThread", NULL);
    
    // run everything on the main thread for Apple applications
    mQueue = dispatch_get_main_queue();
}

SerialDispatchQueueSignaledThread::
~SerialDispatchQueueSignaledThread()
{
    dispatch_release(mQueue);
}

void SerialDispatchQueueSignaledThread::signal()
{
    
}

void SerialDispatchQueueSignaledThread::notify(ProcessCallback_t cb, void * arg)
{
    dispatch_async_f(mQueue, arg, cb);
}

void SerialDispatchQueueSignaledThread::start(bool block)
{
    LOG("Starting!\n");
}

void SerialDispatchQueueSignaledThread::stop()
{
    LOG("Stopping!\n");
}
