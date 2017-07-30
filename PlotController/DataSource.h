#ifndef __DATA_SOURCE_H__
#define __DATA_SOURCE_H__

#include <stdlib.h>

struct Point
{
    float x;
    float y;
};

typedef struct Point Point;

class DataSource
{
public:
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
};

#endif
