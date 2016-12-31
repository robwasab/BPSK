#ifndef __PLOT_CONTROLLER_H__
#define __PLOT_CONTROLLER_H__

#include "DataSource.h"

class PlotController
{
public:
    PlotController(int argc, char ** argv);
    void add_plot(DataSource * source);
    void remove_plot(DataSource * source);
    int run();
private:
    void * main_window;
    void * application;
};

#endif
