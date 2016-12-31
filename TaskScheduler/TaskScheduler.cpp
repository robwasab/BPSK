#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "TaskScheduler.h"
#include "../Module/Module.h"

TaskScheduler::TaskScheduler(size_t max_events) : events(max_events)
{
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    quit = false;
}

int TaskScheduler::add_event(Runnable runnable, void * arg, bool pause)
{
    Event e;
    e.run = runnable;
    e.arg = arg;
    e.pause = pause;
    e.type = RUNNABLE;
    e.module = NULL;
    e.block = NULL;
    Event * ret = events.add(e);
    signal();
    return ret ? 0 : -1;
}

int TaskScheduler::add_module(Module * module, Block * block, bool pause)
{
    Event e;
    e.module = module;
    e.block = block;
    e.pause = pause;
    e.type = MODULE;
    e.run = NULL;
    e.arg = NULL;
    Event * ret = events.add(e);
    //printf("signaling myself\n");
    signal();
    return ret ? 0 : -1;
}

int TaskScheduler::add_event(Runnable runnable, void * arg)
{
    return add_event(runnable, arg, false);
}

int TaskScheduler::add_module(Module * module, Block * block)
{
    return add_module(module, block, false);
}

int TaskScheduler::run_event()
{
    Event e;
    int rc = events.get(&e);

    if (rc) return rc;

    switch(e.type)
    {
        case RUNNABLE:
        e.run(e.arg);
        break;

        case MODULE:
        // GREEN;
        // printf("Running %s...\n", e.module->name());
        // ENDC;
        Block * ret = e.module->process(e.block);
        if (ret && e.module->next)
        {
            // GREEN;
            // printf("Passing to %s...\n", e.module->next->name());
            // ENDC;
            add_module(e.module->next, ret);
        }
        else 
        {
            if (ret && !e.module->next) 
            {
                BLUE;
                printf("Next module is NULL, dumping data!\n");
                ENDC;
                ret->reset();
                float ** iter = ret->get_iterator();
                do 
                {
                    printf("%.3f\n", **iter);
                } while(ret->next());
                ret->free();
            }
            else 
            {
                RED;
                printf("Module returned NULL...\n");
                ENDC;
            }
        }
        break;
    }
    return 0;
}

void TaskScheduler::stop()
{
    this->lock();
    {
        quit = true;
    }
    this->unlock();
    signal();
    pthread_join(main, NULL);
}

void TaskScheduler::lock()
{
    pthread_mutex_lock(&mutex);
}

void TaskScheduler::unlock()
{
    pthread_mutex_unlock(&mutex);
}

void TaskScheduler::signal()
{
    this->lock();
    {
        pthread_cond_signal(&cond);  
    }
    this->unlock();
}

void * task_loop(void * args)
{
    TaskScheduler * self = (TaskScheduler *) args;
    bool quit = false;
    while (1)
    {
        self->lock();
        {
            quit = self->quit;
        }
        self->unlock();

        if (quit) 
        {
            break;
        }

        //printf("Checking size!\n");
        if (self->events.size() > 0) 
        {
            self->run_event();
        }
        else 
        {
            printf("going to sleep...\n");
            self->lock();
            pthread_cond_wait(&self->cond, &self->mutex);
            self->unlock();
            printf("waking up!\n");
        }
    }
    return NULL;
}

void TaskScheduler::start()
{
    pthread_create(&main, NULL, task_loop, this);
}
