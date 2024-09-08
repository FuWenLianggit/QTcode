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
class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget* parent = nullptr);

    void setSymbol(QwtSymbol* symbol);
    void setSamples( QwtPlotCurve* curve,const QVector< QPointF >& samples );
    // void mouseDoubleClickEvent(QMouseEvent* event);
private:
    QwtPlotCurve* m_curve;
    QLabel* m_infoLabel;  // 用于显示点信息的标签



    class HoverPicker : public QwtPlotPicker
    {
    public:
        HoverPicker(QWidget* canvas, QwtPlotCurve* curve, QLabel* infoLabel);

        QwtText trackerTextF(const QPointF& pos) const override;

    protected:
        void widgetMousePressEvent(QMouseEvent* e) override;

    private:
        QwtPlotCurve* m_curve;
        QLabel* m_infoLabel;  // 用于显示点信息的标签
        QPointF m_lastClickedPoint;
        // QElapsedTimer m_lastClickTime;  // 使用 QElapsedTimer 替代 QTime
        QTime m_lastClickTime;
        QPointF first_node=QPointF(3.14, 3.14);;
        QPointF second_node = QPointF(3.14, 3.14);;
    };
};
