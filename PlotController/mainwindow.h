#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <qmainwindow.h>
#include <qlayout.h>
#include <list>
#include <queue>
#include "DataSource.h"
#include "PlotController.h"

class Plot;
class Panel;
class QLabel;
class Settings;

using namespace std;

struct Task 
{
    enum { ADD_PLOT, REMOVE_PLOT } action;
    DataSource * source;
};

typedef struct Task Task;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = NULL);
    virtual bool eventFilter( QObject *, QEvent * );
    void add_plot( DataSource * source );
    void remove_plot( DataSource * source );
    void __add_plot__( DataSource * source );
    void __remove_plot__( DataSource * source );
    void closeEvent ( QCloseEvent * event );
    void set_close_cb(OnCloseCallback cb, void * obj);

private Q_SLOTS:
    void applySettings( const Settings & );

private:
    list <Plot *> plots;
    queue <Task> tasks;
    QMutex mutex;

    QHBoxLayout * layout;
    QWidget * w;
    QLabel *d_frameCount;
    bool checkCanvas( QObject * object );

    OnCloseCallback cb;
    void * obj;

protected:
    void timerEvent( QTimerEvent * event );
};

#endif
