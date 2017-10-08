#include <qapplication.h>
#include "QTPlotController.h"
#include "mainwindow.h"

QTPlotController::QTPlotController(int argc, char ** argv)
{
    application = (void *) new QApplication( argc, argv );
    main_window = (void *) new MainWindow();
}

void QTPlotController::add_plot(DataSource * source)
{
    MainWindow * mw = (MainWindow *) main_window;
    mw->add_plot(source);
}

void QTPlotController::remove_plot(DataSource * source)
{
    MainWindow * mw = (MainWindow *) main_window;
    mw->remove_plot(source);
}

void QTPlotController::set_close_cb(OnCloseCallback cb, void * obj)
{
    MainWindow * mw = (MainWindow *) main_window;
    mw->set_close_cb(cb, obj);
}

int QTPlotController::run()
{
    QApplication * app = (QApplication *) application;
    MainWindow * mw = (MainWindow *) main_window;
    mw->show();
    return app->exec();
}
