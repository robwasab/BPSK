#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_plot.h>
#include <qwt_system_clock.h>
#include "settings.h"
#include "DataSource.h"

class QwtPlotGrid;
class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget* = NULL, DataSource * source = NULL);
    ~Plot();
    DataSource * source;

public Q_SLOTS:
    void setSettings( const Settings & );

protected:
    virtual void timerEvent( QTimerEvent *e );

    /* Override QWidget closeEvent */
    void closeEvent(QCloseEvent * event);

private:
    void alignScales();
    void set_updateInterval(int interval);

    int interval;
    QwtPlotGrid *d_grid;
    QwtPlotCurve *d_curve;

    QwtSystemClock d_clock;
    double d_interval;

    int d_timerId;

    Settings d_settings;
};

#endif
