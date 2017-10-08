#ifndef __PLOT_CONTROLLER_H__
#define __PLOT_CONTROLLER_H__

#include "DataSource.h"

typedef void (*OnCloseCallback)(void * obj);

class PlotController
{
public:
    virtual ~PlotController() {}
    virtual void add_plot(DataSource * source) = 0;
    virtual void remove_plot(DataSource * source) = 0;
    virtual void set_close_cb(OnCloseCallback cb, void * obj) = 0;
    virtual int run() = 0;
};

#endif
