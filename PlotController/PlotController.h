#ifndef __PLOT_CONTROLLER_H__
#define __PLOT_CONTROLLER_H__

#include "DataSource.h"

typedef void (*OnCloseCallback)(void * obj);

class PlotController
{
public:
    PlotController(int argc, char ** argv);
    void add_plot(DataSource * source);
    void remove_plot(DataSource * source);
    void set_close_cb(OnCloseCallback cb, void * obj);
    int run();
private:
    void * main_window;
    void * application;
};

#endif
