#ifndef __TASK_SCHEDULER_H__
#define __TASK_SCHEDULER_H__

#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Queue/Queue.h"
#include "../Module/Module.h"

typedef int (*Runnable)(void * args);

typedef enum {RUNNABLE, MODULE} Type;

struct Event {
    Type type;
    Module * module;
    Block * block;
    Runnable run;
    void * arg;
    bool pause;
};

typedef struct Event Event;

class TaskScheduler
{
public:
    TaskScheduler(size_t max_events);
    int work();
    int add_event(Runnable runnable, void * arg);
    int add_event(Runnable runnable, void * arg, bool pause);
    int add_module(Module * module, Block * block);
    int add_module(Module * module, Block * block, bool pause);
    int run_event();
    void start();
    void stop();
    void lock();
    void unlock();
    bool quit;
    Queue<Event> events;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
private:
    pthread_t main;
    struct timespec time_start;
    struct timespec time_stop;
    void signal();
    void tic();
    void tok();
    uint64_t get_time();
};

#endif
