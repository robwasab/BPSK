#include <qglobal.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_magnifier.h>
#include <stdio.h>
#include "plot.h"
#include "circularbuffer.h"
#include "settings.h"

/*
static double wave( double x )
{
    const double period = 1.0;
    const double c = 5.0;

    double v = ::fmod( x, period );

    const double amplitude = qAbs( x - qRound( x / c ) * c ) / ( 0.5 * c );
    v = amplitude * qSin( v / period * 2 * M_PI );

    return v;
}
*/

class DataSourceWrapper: public QwtSeriesData<QPointF>
{
public:
    DataSourceWrapper( DataSource * source ) {
        this->source = source;
    }

    virtual size_t size() const {
        return source->size();
    }

    virtual QPointF sample( size_t i ) const {
        Point pnt = source->get_data(i);
        return QPointF(pnt.x, pnt.y);
    }

    virtual QRectF boundingRect() const {
        //return QRectF( -1.25, 0, 2.5, source->size() );
        Point origin = source->get_origin();
        Point length = source->get_lengths();
        return QRectF(origin.y, origin.x, length.y, length.x);
    }

private:
    DataSource * source;
};


Plot::Plot( QWidget * parent, DataSource * source ):
    QwtPlot( parent ),
    source( source ),
    d_interval( 10.0 ), // seconds
    d_timerId( -1 )
{
    if ( source ) {
        source->plot = (void *) this;
    }

    // Assign a title
    setTitle( source->name() );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFrameStyle( QFrame::Box | QFrame::Plain );
    canvas->setLineWidth( 1 );
    canvas->setPalette( Qt::white );

    setCanvas( canvas );

    QwtPlotMagnifier* magnifier = new QwtPlotMagnifier(canvas);
    magnifier->setMouseButton(Qt::LeftButton);

    // Insert grid
    d_grid = new QwtPlotGrid();
    d_grid->attach( this );

    // Insert curve
    d_curve = new QwtPlotCurve( "Data Moving Right" );
    d_curve->setPen( Qt::black );
    //d_curve->setData( new CircularBuffer( d_interval, 10 ) );
    d_curve->setData( new DataSourceWrapper( source ) );
    d_curve->attach( this );

    // Axis
    Point origin = source->get_origin();
    Point length = source->get_lengths();

    setAxisTitle( QwtPlot::xBottom, source->xlabel() );
    setAxisScale( QwtPlot::xBottom, origin.x, origin.x + length.x );

    setAxisTitle( QwtPlot::yLeft, source->ylabel() );
    setAxisScale( QwtPlot::yLeft, origin.y, origin.y + length.y );

    d_clock.start();

    d_settings.updateInterval = source->get_updateInterval();
    d_settings.curve.numPoints = source->size();
    d_settings.canvas.useBackingStore = false;

    alignScales();

    setSettings( d_settings );
}

//
//  Set a plain canvas frame and align the scales to it
//
void Plot::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = axisWidget( i );
        if ( scaleWidget )
            scaleWidget->setMargin( 0 );

        QwtScaleDraw *scaleDraw = axisScaleDraw( i );
        if ( scaleDraw )
            scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    }
    plotLayout()->setAlignCanvasToScales( true );
}

void Plot::set_updateInterval(int interval)
{
    if ( interval != this->interval ) {
        this->interval = interval;

        if ( d_timerId >= 0 ) {
            killTimer( d_timerId );
        }
        d_timerId = startTimer( interval );
    }
}

void Plot::setSettings( const Settings &s )
{
    if ( d_timerId >= 0 )
        killTimer( d_timerId );

    d_timerId = startTimer( s.updateInterval );

    d_grid->setPen( s.grid.pen );
    d_grid->setVisible( s.grid.pen.style() != Qt::NoPen );

    /*
    CircularBuffer *buffer = static_cast<CircularBuffer *>( d_curve->data() );
    if ( s.curve.numPoints != buffer->size() ||
            s.curve.functionType != d_settings.curve.functionType )
    {
        switch( s.curve.functionType )
        {
            case Settings::Wave:
                buffer->setFunction( wave );
                break;
            case Settings::Noise:
                buffer->setFunction( noise );
                break;
            default:
                buffer->setFunction( NULL );
        }

        buffer->fill( d_interval, s.curve.numPoints );
    }
    */

    d_curve->setPen( s.curve.pen );
    d_curve->setBrush( s.curve.brush );

    d_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons,
        s.curve.paintAttributes & QwtPlotCurve::ClipPolygons );
    d_curve->setPaintAttribute( QwtPlotCurve::FilterPoints,
        s.curve.paintAttributes & QwtPlotCurve::FilterPoints );

    d_curve->setRenderHint( QwtPlotItem::RenderAntialiased,
        s.curve.renderHint & QwtPlotItem::RenderAntialiased );

    QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( canvas() );
    if ( plotCanvas == NULL )
    {
        plotCanvas = new QwtPlotCanvas();
        plotCanvas->setFrameStyle( QFrame::Box | QFrame::Plain );
        plotCanvas->setLineWidth( 1 );
        plotCanvas->setPalette( Qt::white );

        setCanvas( plotCanvas );
    }

    plotCanvas->setAttribute( Qt::WA_PaintOnScreen, s.canvas.paintOnScreen );

    plotCanvas->setPaintAttribute(
        QwtPlotCanvas::BackingStore, s.canvas.useBackingStore );
    plotCanvas->setPaintAttribute(
        QwtPlotCanvas::ImmediatePaint, s.canvas.immediatePaint );

    QwtPainter::setPolylineSplitting( s.curve.lineSplitting );

    d_settings = s;
}

void Plot::timerEvent( QTimerEvent * )
{
    //CircularBuffer *buffer = static_cast<CircularBuffer *>( d_curve->data() );
    //buffer->setReferenceTime( d_clock.elapsed() / 100.0 );
    if (!source->valid()) {
        Point origin = source->get_origin();
        Point length = source->get_lengths();
        float pad = 0.1 * length.y;
        setAxisScale( QwtPlot::xBottom, origin.x, origin.x + length.x );
        setAxisScale( QwtPlot::yLeft, origin.y - pad, origin.y + length.y + pad);

        set_updateInterval(source->get_updateInterval());
        updateAxes();
    }
    source->next();

    if ( d_settings.updateType == Settings::RepaintCanvas )
    {
        // the axes in this example doesn't change. So all we need to do
        // is to repaint the canvas.

        QMetaObject::invokeMethod( canvas(), "replot", Qt::DirectConnection );
    }
    else
    {
        replot();
    }
}
