#ifndef __SIGNALED_THREAD_H__
#define __SIGNALED_THREAD_H__
#include <stdlib.h>
#include <pthread.h>
#include "../Log/Log.h"
#include "../Queue/Queue.h"

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
        signal();
    }

    /* start the thread */
    virtual void start(bool block)
    {
        pthread_create(&main, NULL, task_loop, this);
        if (block) 
        {
            pthread_join(main, NULL);
        }
    }

    /* stop the thread */
    virtual void stop()
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
        lock();
        pthread_cond_wait(&cond, &mutex);
        unlock();
    }

    static
    void * task_loop(void * args)
    {
        SignaledThread * self = (SignaledThread *) args;

        while(1)
        {
            if (self->notifications.size() > 0)
            {
                /* dispatch event */
                T t;
                int rc;
                rc = self->notifications.get(&t);

                if (!rc)
                {
                    self->process(t);
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

    void signal()
    {
        lock();
        pthread_cond_signal(&cond);
        //pthread_cond_broadcast(&cond);
        unlock();
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
