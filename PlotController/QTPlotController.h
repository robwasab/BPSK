#ifndef __QT_PLOT_CONTROLLER_H__
#define __QT_PLOT_CONTROLLER_H__

#include "DataSource.h"
#include "PlotController.h"

class QTPlotController : public PlotController
{
public:
    QTPlotController(int argc, char ** argv);
    virtual void add_plot(DataSource * source);
    virtual void remove_plot(DataSource * source);
    virtual void set_close_cb(OnCloseCallback cb, void * obj);
    virtual int run();
private:
    void * main_window;
    void * application;
};

#endif
