#include <qstatusbar.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qevent.h>
#include <qdatetime.h>
#include <qwt_plot_canvas.h>
#include <queue>
#include "plot.h"
#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow( QWidget *parent ):
    QMainWindow( parent ),
    plots()
{
    this->w = new QWidget( this );

    this->layout = new QHBoxLayout( w );

    this->setCentralWidget( w );

    this->d_frameCount = new QLabel( this );

    this->statusBar()->addWidget( d_frameCount, 10 );

    this->cb = NULL;
    this->obj = NULL;
    startTimer(500);
}

void MainWindow::set_close_cb(OnCloseCallback cb, void * obj)
{
    this->cb = cb;
    this->obj = obj;
}

void MainWindow::closeEvent( QCloseEvent * event )
{
    if (this->cb != NULL && this->obj != NULL)
    {
        cb(obj);
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::add_plot( DataSource * source )
{
    Task task;
    task.action = Task::ADD_PLOT;
    task.source = source;
    mutex.lock();
    {
        tasks.push(task);
    }
    mutex.unlock();
}

void MainWindow::remove_plot( DataSource * source )
{
    Task task;
    task.action = Task::REMOVE_PLOT;
    task.source = source;
    mutex.lock();
    {
        tasks.push(task);
    }
    mutex.unlock();
}

void MainWindow::__add_plot__( DataSource * source )
{
    Plot * plot = new Plot( this->w, source );
    {
        //plot->canvas()->removeEventFilter( this );
        //plot->canvas()->installEventFilter( this );
    }
    this->plots.push_back( plot );
    this->layout->addWidget( plot );
}

void MainWindow::__remove_plot__( DataSource * source )
{
    list<Plot *>::iterator it;
    for (it = plots.begin(); it != plots.end(); ++it)
    {
        if ((*it)->source == source) 
        {
            layout->removeWidget(*it);
            (*it)->deleteLater();
            plots.erase(it);
        }
    }
}

void MainWindow::timerEvent( QTimerEvent * event )
{
    QWidget::timerEvent( event );

    mutex.lock();
    if (!tasks.empty()) 
    {
        Task& task = tasks.front();
        switch (task.action)
        {
            case Task::ADD_PLOT:
                __add_plot__( task.source );
                break;
            case Task::REMOVE_PLOT:
                __remove_plot__( task.source );
                break;
            default:
                break;
        }
        //adjustSize();
        tasks.pop();
    }
    mutex.unlock();
}

bool MainWindow::checkCanvas( QObject * object)
{
    for (Plot * plot : this->plots) {
        if (plot->canvas() == object) {
            return true;
        }
    }
    return false;
}


bool MainWindow::eventFilter( QObject *object, QEvent *event )
{
    if ( this->checkCanvas(object) && event->type() == QEvent::Paint )
    {
        static int counter;
        static QTime timeStamp;

        if ( !timeStamp.isValid() )
        {
            timeStamp.start();
            counter = 0;
        }
        else
        {
            counter++;

            const double elapsed = timeStamp.elapsed() / 1000.0;
            if ( elapsed >= 1 )
            {
                QString fps;
                fps.setNum( qRound( counter / elapsed ) );
                fps += " Fps";

                d_frameCount->setText( fps );

                counter = 0;
                timeStamp.start();
            }
        }
    }
    return QMainWindow::eventFilter( object, event );
}

void MainWindow::applySettings( const Settings& settings)
{
    (void) settings;
}
