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

// 定义 HoverPicker 类
class HoverPicker : public QwtPlotPicker
{
    Q_OBJECT

public:
    HoverPicker(QWidget* canvas, QwtPlotCurve* curve, QLabel* infoLabel, QStringList &timelist);

    QwtText trackerTextF(const QPointF& pos) const override;

signals:
    void startandendtime(QString starttime, QString endtime);

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
    Plot(QWidget* parent = nullptr);

    void setSymbol(QwtSymbol* symbol);
    void setSamples(QwtPlotCurve* curve, const QVector<QPointF>& samples);
    void setSamplesLIN(QwtPlotCurve* curve, const QVector<QPointF>& samples, QStringList &timelist);
    void setSamplesTipper(QwtPlotCurve* curve);
    void setSamplesLINtime(QString starttime, QString endtime);

signals:
    void LINtime(QString starttime, QString endtime);

private:
    QwtPlotCurve* m_curve;
    QLabel* m_infoLabel;  // 用于显示点信息的标签
    QStringList timeslist;

};
