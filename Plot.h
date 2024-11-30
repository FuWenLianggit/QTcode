/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QElapsedTimer>
#include <Qwt/qwt.h>
#include <QLabel>
#include <Qwt/qwt_plot.h>
#include <qwt_plot_picker.h>  // 这行代码确保包含了 QwtPlotPicker 的声明
#include <qwt_plot_curve.h>
#include <QTime>
class QwtPlotSpectrogram;
// 定义 HoverPicker 类
class HoverPicker : public QwtPlotPicker
{
    Q_OBJECT

public:
    HoverPicker(QWidget* canvas, QwtPlotCurve* curve, QLabel* infoLabel, QStringList &timelist);

    QwtText trackerTextF(const QPointF& pos) const override;

signals:
    void startandendtime(QString starttime, QString endtime,int text1position,int text2position );

protected:
    void widgetMousePressEvent(QMouseEvent* e) override;

private:
    QwtPlotCurve* m_curve;
    QLabel* m_infoLabel;  // 用于显示点信息的标签
    QPointF m_lastClickedPoint;
    QTime m_lastClickTime;
    QStringList &timeslists;
    QPointF first_node = QPointF(3.14, 3.14);
    QPointF second_node = QPointF(3.14, 3.14);
    int timeslistsize;
};

// 定义 Plot 类
class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot(bool flag , QWidget* parent = nullptr);

    void setSymbol(QwtSymbol* symbol);
    void setSamples(QwtPlotCurve* curve, const QVector<QPointF>& samples);
    void setSamplesLIN(QwtPlotCurve* curve, const QVector<QPointF>& samples, QStringList &timelist);
    void setSamplesTipper( QwtPlotCurve* curve,const QVector< QPointF >& samples );
    void setSamplesLINtime(QString starttime, QString endtime,int text1position,int text2position);

signals:
    void LINtime(QString starttime, QString endtime,int text1position,int text2position);

private:
    QwtPlotCurve* m_curve;
    QLabel* m_infoLabel;  // 用于显示点信息的标签
    QStringList timeslist;

// public:
//     enum ColorMap
//     {
//         RGBMap,
//         HueMap,
//         SaturationMap,
//         ValueMap,
//         SVMap,
//         AlphaMap
//     };



// Q_SIGNALS:
//     void rendered( const QString& status );

// public Q_SLOTS:
//     void showContour( bool on );
//     void showSpectrogram( bool on );

//     void setColorMap( int );
//     void setColorTableSize( int );
//     void setAlpha( int );

// // #ifndef QT_NO_PRINTER
// //     void printPlot();
// // #endif

// private:
//     virtual void drawItems( QPainter*, const QRectF&,
//                            const QwtScaleMap maps[QwtAxis::AxisPositions] ) const QWT_OVERRIDE;

//     QwtPlotSpectrogram* m_spectrogram;

//     int m_mapType;
//     int m_alpha;
};
