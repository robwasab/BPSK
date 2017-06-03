#ifndef __SIGNALED_THREAD_H__
#define __SIGNALED_THREAD_H__
#include <stdlib.h>
#include <pthread.h>
#include "../Queue/Queue.h"
#include "../Notify/Notify.h"
#include "../Module/Module.h"

template <class T>
class SignaledThread
{
public:
    SignaledThread(size_t max_notifications=128):
        notifications(max_notifications),
        quit(false)
    {
        pthread_cond_init(&cond, NULL);
        pthread_mutex_init(&mutex, NULL);
    }

    virtual ~SignaledThread() 
    {
    }

    /* add data to the queue, and wakeup the thread for processing */ 
    void notify(T n)
    {
        notifications.add(n);
        LOG("NOTIFYING!\n");
        signal();
    }

    /* start the thread */
    void start(bool block)
    {
        pthread_create(&main, NULL, task_loop, this);
        if (block) 
        {
            pthread_join(main, NULL);
        }
    }

    /* stop the thread */
    void stop()
    {
        lock();
        quit = true;
        unlock();
        signal();
        pthread_join(main, NULL);
    }

    /* should quit? */
    bool should_quit()
    {
        bool should_quit;
        lock();
        should_quit = quit;
        unlock();
        return should_quit;
    }

    /* wait until signal */
    void wait()
    {
        LOG("WAITING!\n");
        lock();
        pthread_cond_wait(&cond, &mutex);
        unlock();
        LOG("WAKING UP!\n");
    }

    static
    void * task_loop(void * args)
    {
        SignaledThread * self = (SignaledThread *) args;


        while( ! self->should_quit() )
        {
            if (self->notifications.size() > 0)
            {
                LOG("DISPATCHING!\n");
                /* dispatch event */
                T t;
                int rc;
                rc = self->notifications.get(&t);

                if (!rc)
                {
                    self->process(t);
                }
            }
            else
            {
                self->wait();
            }
        }
        return NULL;
    }

    virtual void process(T t) = 0;

    /* Member Variables */
    Queue<T> notifications;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
protected:
    bool quit;
    pthread_t main;

    /* Operating system dependent thread functions */
    void signal()
    {
        lock();
        pthread_cond_signal(&cond);
        unlock();
    }

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }

};
#endif
