/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "Plotcontour.h"
#include <qwt_clipper.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include<qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_interval.h>
#include <qwt_painter.h>
#include <QPainter>
#include <qwt_plot_magnifier.h>
#include <QPen>
#include <QtPrintSupport/QPrintDialog>
#include <QElapsedTimer>

namespace
{
class MyZoomer : public QwtPlotZoomer
{
public:
    MyZoomer( QWidget* canvas )
        : QwtPlotZoomer( canvas )
    {
        setTrackerMode( AlwaysOn );
    }

    virtual QwtText trackerTextF( const QPointF& pos ) const QWT_OVERRIDE
    {
        QColor bg( Qt::white );
        bg.setAlpha( 200 );

        QwtText text = QwtPlotZoomer::trackerTextF( pos );
        text.setBackgroundBrush( QBrush( bg ) );
        return text;
    }


};

class SpectrogramData : public QwtRasterData
{
public:
    SpectrogramData(QList<double>& xblocks, QList<double>& yblocks, const QVector<QVector<double>>& data)
        : m_xblocks(xblocks), m_yblocks(yblocks), m_data(data)
    {
        // 计算 x 和 y 坐标
        // 计算 x 和 y 坐标
        // QList<double> x_coords ;  // 初始 0
        // QList<double> y_coords;  // 初始 0

        // // 对 xblocks 和 yblocks 进行累加，生成 x_coords 和 y_coords
        // double cumulative_x = 0;
        // for (double x : xblocks) {
        //     cumulative_x += x;
        //     x_coords.push_back(cumulative_x);
        // }

        // double cumulative_y = 0;
        // for (double y : yblocks) {
        //     cumulative_y += y;
        //     y_coords.push_back(cumulative_y);
        // }

        // // 调整 x 坐标
        // double half_last_x = x_coords.back() / 2.0;
        // for (size_t i = 0; i < x_coords.size(); ++i) {
        //     x_coords[i] -= half_last_x;
        //     qDebug() << x_coords[i];
        // }
        // qDebug() << m_xblocks.size() << x_coords.size();
        // m_xblocks = x_coords;
        // m_yblocks = y_coords;
        // 设置适当的轴范围
        m_intervals[Qt::XAxis] = QwtInterval(m_xblocks.first(), m_xblocks.last());
        m_intervals[Qt::YAxis] = QwtInterval(m_yblocks.first(), m_yblocks.last());
        double min_resistivity,max_resistivity ;
        // 绘制网格和等值线
        double min_resistivitytemp =10000000,max_resistivitytemp =0 ;
        for (int i = 0; i< data.size();i++){
            min_resistivity = *std::min_element(data[i].begin(), data[i].end());
            max_resistivity = *std::max_element(data[i].begin(), data[i].end());

            if (min_resistivity < min_resistivitytemp) min_resistivitytemp = min_resistivity;
            if (max_resistivity > max_resistivitytemp) max_resistivitytemp = max_resistivity;
        }

        qDebug() << data[0];
        qDebug() << min_resistivitytemp << max_resistivitytemp;
        min_resistivity = min_resistivitytemp;
        max_resistivity = max_resistivitytemp;
        m_intervals[Qt::ZAxis] = QwtInterval(min_resistivity, max_resistivity);  // 假设 Z 轴范围是 0 到 10
    }

    virtual QwtInterval interval(Qt::Axis axis) const QWT_OVERRIDE
    {
        if (axis >= 0 && axis <= 2)
            return m_intervals[axis];
        return QwtInterval();
    }

    virtual double value(double x, double y) const QWT_OVERRIDE
    {
        // 假设数据在 x 和 y 的范围内进行插值
        int ix = std::lower_bound(m_xblocks.begin(), m_xblocks.end(), x) - m_xblocks.begin();
        int iy = std::lower_bound(m_yblocks.begin(), m_yblocks.end(), y) - m_yblocks.begin();

        if (ix < m_xblocks.size() && iy < m_yblocks.size())
            return m_data[iy][ix];

        return 0.0;  // 如果坐标不在数据范围内，返回默认值 0
    }

private:
    QList<double> m_xblocks;
    QList<double> m_yblocks;
    QVector<QVector<double>> m_data;  // 二维数据
    QwtInterval m_intervals[3];
};

class LinearColorMap : public QwtLinearColorMap
{
public:
    LinearColorMap( int formatType )
        : QwtLinearColorMap( Qt::darkCyan, Qt::red )
    {
        setFormat( ( QwtColorMap::Format ) formatType );

        addColorStop( 0.1, Qt::cyan );
        addColorStop( 0.6, Qt::green );
        addColorStop( 0.95, Qt::yellow );
    }
};

class HueColorMap : public QwtHueColorMap
{
public:
    HueColorMap( int formatType )
        : QwtHueColorMap( QwtColorMap::Indexed )
    {
        setFormat( ( QwtColorMap::Format ) formatType );

        //setHueInterval( 240, 60 );
        //setHueInterval( 240, 420 );
        setHueInterval( 0, 359 );
        setSaturation( 150 );
        setValue( 200 );
    }
};

class SaturationColorMap : public QwtSaturationValueColorMap
{
public:
    SaturationColorMap( int formatType )
    {
        setFormat( ( QwtColorMap::Format ) formatType );

        setHue( 220 );
        setSaturationInterval( 0, 255 );
        setValueInterval( 255, 255 );
    }
};

class ValueColorMap : public QwtSaturationValueColorMap
{
public:
    ValueColorMap( int formatType )
    {
        setFormat( ( QwtColorMap::Format ) formatType );

        setHue( 220 );
        setSaturationInterval( 255, 255 );
        setValueInterval( 70, 255 );
    }
};

class SVColorMap : public QwtSaturationValueColorMap
{
public:
    SVColorMap( int formatType )
    {
        setFormat( ( QwtColorMap::Format ) formatType );

        setHue( 220 );
        setSaturationInterval( 100, 255 );
        setValueInterval( 70, 255 );
    }
};

class AlphaColorMap : public QwtAlphaColorMap
{
public:
    AlphaColorMap( int formatType )
    {
        setFormat( ( QwtColorMap::Format ) formatType );

        //setColor( QColor("DarkSalmon") );
        setColor( QColor("SteelBlue") );
    }
};



class Spectrogram : public QwtPlotSpectrogram
{
public:
    int elapsed() const
    {
        return m_elapsed;
    }

    QSize renderedSize() const
    {
        return m_renderedSize;
    }

protected:
    virtual QImage renderImage(
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& area, const QSize& imageSize ) const QWT_OVERRIDE
    {
        QElapsedTimer t;
        t.start();

        QImage image = QwtPlotSpectrogram::renderImage(
            xMap, yMap, area, imageSize );

        m_elapsed = t.elapsed();
        m_renderedSize = imageSize;

        return image;
    }

private:
    mutable int m_elapsed;
    mutable QSize m_renderedSize;
};
}

// 定义函数放大某个区域
void zoomToRegion(QwtPlot* plot, double x_min, double x_max, double y_min, double y_max)
{
    plot->setAxisScale(QwtAxis::XBottom, x_min, x_max); // 设置 X 轴范围
    plot->setAxisScale(QwtAxis::YLeft, y_min, y_max);   // 设置 Y 轴范围
    plot->replot();                                    // 重新绘制图像
}

Plotcontour::Plotcontour(std::vector<double> xblocks1,std::vector<double> yblocks1, std::vector<std::vector<double>> data1,  QWidget* parent )
    : QwtPlot( parent )
    , m_alpha(255),xblocks(xblocks1),yblocks(yblocks1),data(data1)

{
    canvas()->setStyleSheet(
        "border: 2px solid Black;"
        // "border-radius: 15px;"
        "background-color:white;"
        // "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1,"
        // "stop: 0 LemonChiffon, stop: 1 PaleGoldenrod );"
        );

    // (void )new QwtPlotPanner( canvas() );

    // QwtPlotMagnifier* magnifier = new QwtPlotMagnifier( canvas() );
    // magnifier->setMouseButton( Qt::NoButton );

    m_spectrogram = new Spectrogram();
    m_spectrogram->setRenderThreadCount( 0 ); // use system specific thread count
    m_spectrogram->setCachePolicy( QwtPlotRasterItem::PaintCache );

    double min_resistivity,max_resistivity ;
    // 绘制网格和等值线
    double min_resistivitytemp =10000000,max_resistivitytemp =0 ;
    for (int i = 0; i< data.size();i++){
        min_resistivity = *std::min_element(data[i].begin(), data[i].end());
        max_resistivity = *std::max_element(data[i].begin(), data[i].end());

        if (min_resistivity < min_resistivitytemp) min_resistivitytemp = min_resistivity;
        if (max_resistivity > max_resistivitytemp) max_resistivitytemp = max_resistivity;
    }


    min_resistivity = min_resistivitytemp;
    max_resistivity = max_resistivitytemp;

    QList< double > contourLevels;
    for ( double level = min_resistivitytemp; level < max_resistivitytemp; level += 0.01 )
        contourLevels += level;
    m_spectrogram->setContourLevels( contourLevels );
    QList<double> xblockslist(xblocks.begin(), xblocks.end());
    QList<double> yblockslist(yblocks.begin(), yblocks.end());

    QList<QList<double>> datalist;

    for (const auto& innerVec : data) {
        QList<double> qtInnerList;
        std::copy(innerVec.begin(), innerVec.end(), std::back_inserter(qtInnerList));  // 将内层 vector 转换为 QList
        datalist.append(qtInnerList);  // 将内层 QList 添加到外层 QList
    }

    m_spectrogram->setData( new SpectrogramData(xblockslist,yblockslist,datalist) );
    m_spectrogram->attach( this );

    const QwtInterval zInterval = m_spectrogram->data()->interval( Qt::ZAxis );
    // 翻转 Y 轴
    setAxisTitle(QwtPlot::xBottom, "Distance / m"); // 设置横轴名称
    setAxisTitle(QwtPlot::yLeft, "Depth / m");   // 设置纵轴名称
    double minY = *std::min_element(yblocks.begin(), yblocks.end());
    double maxY = *std::max_element(yblocks.begin(), yblocks.end());
    setAxisScale(QwtPlot::yLeft, maxY, 0); // 设置 Y 轴从 maxY 到 minY


    // 示例：放大画布中某个区域
    double zoom_x_min = xblocks[0], zoom_x_max = xblocks.back();
    double zoom_y_min = yblocks.back(), zoom_y_max =0;
    zoomToRegion(this, zoom_x_min, zoom_x_max, zoom_y_min, zoom_y_max);

    // A color bar on the right axis
    QwtScaleWidget* rightAxis = axisWidget( QwtAxis::YRight );
    rightAxis->setTitle( "Intensity" );
    rightAxis->setColorBarEnabled( true );

    setAxisScale( QwtAxis::YRight, zInterval.minValue(), zInterval.maxValue() );
    setAxisVisible( QwtAxis::YRight );

    plotLayout()->setAlignCanvasToScales( true );

    setColorMap( Plotcontour::RGBMap );

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

    // QwtPlotZoomer* zoomer = new MyZoomer( canvas() );
    // zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
    //                         Qt::RightButton, Qt::ControlModifier );
    // zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
    //                         Qt::RightButton );

    // QwtPlotPanner* panner = new QwtPlotPanner( canvas() );
    // panner->setAxisEnabled( QwtAxis::YRight, false );
    // panner->setMouseButton( Qt::MiddleButton );

    // // Avoid jumping when labels with more/less digits
    // // appear/disappear when scrolling vertically

    // const int extent = QwtPainter::horizontalAdvance(
    //     axisWidget( QwtAxis::YLeft )->fontMetrics(), "100.00" );

    // axisScaleDraw( QwtAxis::YLeft )->setMinimumExtent( extent );

    // const QColor c( Qt::darkBlue );
    // zoomer->setRubberBandPen( c );
    // zoomer->setTrackerPen( c );
    // QwtPlotMagnifier* magnifier = new QwtPlotMagnifier( canvas() );
}

void Plotcontour::showContour( bool on )
{
    m_spectrogram->setDisplayMode( QwtPlotSpectrogram::ContourMode, on );
    replot();
}

void Plotcontour::showSpectrogram( bool on )
{
    m_spectrogram->setDisplayMode( QwtPlotSpectrogram::ImageMode, on );
    m_spectrogram->setDefaultContourPen(
        on ? QPen( Qt::black, 0 ) : QPen( Qt::NoPen ) );

    replot();
}

void Plotcontour::setColorTableSize( int type )
{
    int numColors = 0;
    switch( type )
    {
    case 1:
        numColors = 256;
        break;
    case 2:
        numColors = 1024;
        break;
    case 3:
        numColors = 16384;
        break;
    }

    m_spectrogram->setColorTableSize( numColors );
    replot();
}

void Plotcontour::setColorMap( int type )
{
    QwtScaleWidget* axis = axisWidget( QwtAxis::YRight );
    const QwtInterval zInterval = m_spectrogram->data()->interval( Qt::ZAxis );

    m_mapType = type;

    const QwtColorMap::Format format = QwtColorMap::RGB;

    int alpha = m_alpha;
    switch( type )
    {
    case Plotcontour::HueMap:
    {
        m_spectrogram->setColorMap( new HueColorMap( format ) );
        axis->setColorMap( zInterval, new HueColorMap( format ) );
        break;
    }
    case Plotcontour::SaturationMap:
    {
        m_spectrogram->setColorMap( new SaturationColorMap( format ) );
        axis->setColorMap( zInterval, new SaturationColorMap( format ) );
        break;
    }
    case Plotcontour::ValueMap:
    {
        m_spectrogram->setColorMap( new ValueColorMap( format ) );
        axis->setColorMap( zInterval, new ValueColorMap( format ) );
        break;
    }
    case Plotcontour::SVMap:
    {
        m_spectrogram->setColorMap( new SVColorMap( format ) );
        axis->setColorMap( zInterval, new SVColorMap( format ) );
        break;
    }
    case Plotcontour::AlphaMap:
    {
        alpha = 255;
        m_spectrogram->setColorMap( new AlphaColorMap( format ) );
        axis->setColorMap( zInterval, new AlphaColorMap( format ) );
        break;
    }
    case Plotcontour::RGBMap:
    default:
    {
        m_spectrogram->setColorMap( new LinearColorMap( format ) );
        axis->setColorMap( zInterval, new LinearColorMap( format ) );
    }
    }
    m_spectrogram->setAlpha( alpha );

    replot();
}

void Plotcontour::setAlpha( int alpha )
{
    // setting an alpha value doesn't make sense in combination
    // with a color map interpolating the alpha value

    m_alpha = alpha;
    if ( m_mapType != Plotcontour::AlphaMap )
    {
        m_spectrogram->setAlpha( alpha );
        replot();
    }
}

void Plotcontour::drawItems( QPainter* painter, const QRectF& canvasRect,
                     const QwtScaleMap maps[QwtAxis::AxisPositions] ) const
{
    QwtPlot::drawItems( painter, canvasRect, maps );

    if ( m_spectrogram )
    {
        Spectrogram* spectrogram = static_cast< Spectrogram* >( m_spectrogram );

        QString info( "%1 x %2 pixels: %3 ms" );
        info = info.arg( spectrogram->renderedSize().width() );
        info = info.arg( spectrogram->renderedSize().height() );
        info = info.arg( spectrogram->elapsed() );

        Plotcontour* plot = const_cast< Plotcontour* >( this );
        plot->Q_EMIT rendered( info );
    }
}

// #ifndef QT_NO_PRINTER

// void Plotcontour::printPlot()
// {
//     QPrinter printer( QPrinter::HighResolution );
// #if QT_VERSION >= 0x050300
//     printer.setPageOrientation( QPageLayout::Landscape );
// #else
//     printer.setOrientation( QPrinter::Landscape );
// #endif
//     printer.setOutputFileName( "spectrogram.pdf" );

//     QPrintDialog dialog( &printer );
//     if ( dialog.exec() )
//     {
//         QwtPlotRenderer renderer;

//         if ( printer.colorMode() == QPrinter::GrayScale )
//         {
//             renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
//             renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
//             renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
//             renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
//         }

//         renderer.renderTo( this, printer );
//     }
// }

// #endif

// #include "moc_Plot.cpp"
