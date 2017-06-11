#include <qapplication.h>
#include "PlotController.h"
#include "mainwindow.h"

PlotController::PlotController(int argc, char ** argv)
{
    application = (void *) new QApplication( argc, argv );
    main_window = (void *) new MainWindow();
}

void PlotController::add_plot(DataSource * source)
{
    MainWindow * mw = (MainWindow *) main_window;
    mw->add_plot(source);
}

void PlotController::remove_plot(DataSource * source)
{
    MainWindow * mw = (MainWindow *) main_window;
    mw->remove_plot(source);
}

void PlotController::set_close_cb(OnCloseCallback cb, void * obj)
{
    MainWindow * mw = (MainWindow *) main_window;
    mw->set_close_cb(cb, obj);
}

int PlotController::run()
{
    QApplication * app = (QApplication *) application;
    MainWindow * mw = (MainWindow *) main_window;
    mw->show();
    return app->exec();
}
