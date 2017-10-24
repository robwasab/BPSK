#ifndef __DATA_SOURCE_H__
#define __DATA_SOURCE_H__

#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define NUM_FRAMES 6
#define UPDATE_RATE_HZ 12
#define UPDATE_INTERVAL_MS (1000/UPDATE_RATE_HZ)
#define FRAME_SIZE(fs) ((int)(round(1.0*fs/UPDATE_RATE_HZ)))
#define BUFFER_SIZE(fs) (NUM_FRAMES * FRAME_SIZE(fs))

struct AFPoint
{
    float x;
    float y;
};

typedef struct AFPoint AFPoint;

class DataSource
{
public:
    DataSource()
    {
        req_quit = false;
        quit_ack = false;
        pthread_mutex_init(&mutex, NULL);
    }

    virtual ~DataSource()
    {
    }
    virtual size_t size() = 0;
    virtual AFPoint get_data(size_t index) = 0;
    virtual void next() = 0;
    void * plot;
    virtual AFPoint get_origin() = 0;
    virtual AFPoint get_lengths() = 0;
    virtual bool valid() = 0;

    /* return milliseconds */
    virtual int get_updateInterval()
    {
        return UPDATE_INTERVAL_MS;
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
