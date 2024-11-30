/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <Qwt/qwt.h>
#include <Qwt/qwt_plot.h>
class QwtPlotSpectrogram;

class Plotcontour : public QwtPlot
{
    Q_OBJECT

public:
    enum ColorMap
    {
        RGBMap,
        HueMap,
        SaturationMap,
        ValueMap,
        SVMap,
        AlphaMap
    };

    Plotcontour(std::vector<double> xblocks,std::vector<double> yblocks, std::vector<std::vector<double>> data, QWidget* = NULL );

Q_SIGNALS:
    void rendered( const QString& status );

public Q_SLOTS:
    void showContour( bool on );
    void showSpectrogram( bool on );

    void setColorMap( int );
    void setColorTableSize( int );
    void setAlpha( int );

// #ifndef QT_NO_PRINTER
//     void printPlot();
// #endif

private:
    virtual void drawItems( QPainter*, const QRectF&,
                           const QwtScaleMap maps[QwtAxis::AxisPositions] ) const QWT_OVERRIDE;

    QwtPlotSpectrogram* m_spectrogram;

    int m_mapType;
    int m_alpha;
    std::vector<double> xblocks;
    std::vector<double> yblocks;
    std::vector<std::vector<double>> data;
};
