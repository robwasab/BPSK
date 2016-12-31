#ifndef __PLOT_QUEUE_H__
#define __PLOT_QUEUE_H__

#include <qwt_series_data.h>

template <class Type> class PlotQueue: public QwtSeriesData<QPointF>
{
public:
    PlotQueue( size_t size );
    
