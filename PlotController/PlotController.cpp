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

int PlotController::run()
{
    QApplication * app = (QApplication *) application;
    MainWindow * mw = (MainWindow *) main_window;
    mw->show();
    return app->exec();
}
