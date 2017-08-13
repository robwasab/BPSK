#ifndef __DATA_SOURCE_H__
#define __DATA_SOURCE_H__

#include <stdlib.h>
#include <pthread.h>

struct Point
{
    float x;
    float y;
};

typedef struct Point Point;

class DataSource
{
public:
    DataSource()
    {
        req_quit = false;
        quit_ack = false;
        pthread_mutex_init(&mutex, NULL);
    }

    virtual size_t size() = 0;
    virtual Point get_data(size_t index) = 0;
    virtual void next() = 0;
    void * plot;
    virtual Point get_origin() = 0;
    virtual Point get_lengths() = 0;
    virtual bool valid() = 0;

    /* return milliseconds */
    virtual int get_updateInterval()
    {
        return 50;
    }

    virtual const char * name() = 0;

    virtual const char * xlabel()
    {
        static char text[] = "Index";
        return text;
    }

    virtual const char * ylabel()
    {
        static char text[] = "Amplitude";
        return text;
    }

    void join()
    {
        while (true)
        {
            pthread_mutex_lock(&mutex);
            if (quit_ack)
            {
                pthread_mutex_unlock(&mutex);
                break;
            }
            pthread_mutex_unlock(&mutex);
        }
    }

    /* Used by the radio to indicate the intent to quit */
    void requeust_quit()
    {
        pthread_mutex_lock(&mutex);
        req_quit = true;
        pthread_mutex_unlock(&mutex);
    }

    /* Used by the Plot controller to inquire whether or not a quit had been requeusted */
    bool quit_requested()
    {
        bool ret;
        pthread_mutex_lock(&mutex);
        ret = req_quit;
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    /* Used by the Plot controller to acknowledge quit */
    void acknowledge_quit()
    {
        pthread_mutex_lock(&mutex);
        quit_ack = true;
        pthread_mutex_unlock(&mutex);
    }

private:
    pthread_mutex_t mutex;
    bool req_quit;
    bool quit_ack;
};

#endif
