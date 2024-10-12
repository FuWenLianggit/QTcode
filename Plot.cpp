/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "Plot.h"
#include <qwt_plot_magnifier.h>
#include <QMouseEvent>
#include <QTime>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_curve.h>
#include <qwt_text.h>
#include <QPen>
#include <QApplication>
#include <QMessageBox>
#include <qwt_symbol.h>
// HoverPicker 类的实现
 HoverPicker::HoverPicker(QWidget* canvas, QwtPlotCurve* curve, QLabel* infoLabel,QStringList & timelist)
    : QwtPlotPicker(canvas), m_curve(curve), m_infoLabel(infoLabel),timeslist(timelist)
{
    setTrackerMode(QwtPicker::AlwaysOn);
    setRubberBand(QwtPicker::NoRubberBand);
    m_lastClickedPoint = QPointF(); // 初始化最后点击点
}

QwtText  HoverPicker::trackerTextF(const QPointF& pos) const
{
    if (!m_curve || m_curve->dataSize() == 0)
    {
        qDebug() << "m_curve is not valid or has no data";
        return QwtText();
    }


    double minDist = 10 ; // 使用最小距离阈值
    QPointF closestPoint;
    bool pointFound = false;

    for (int i = 0; i < m_curve->dataSize(); ++i)
    {
        QPointF sample = m_curve->sample(i);
        double dist = QLineF(pos, sample).length();
        if (dist < minDist)
        {
            // minDist = dist;
            closestPoint = sample;
            pointFound = true;
        }
    }

    if (pointFound)
    {
        // QString text = QString("X: %1\nY: %2")
        // .arg(closestPoint.x())
        //     .arg(closestPoint.y());

        // QwtText qwtText(text);
        // qwtText.setBackgroundBrush(QBrush(Qt::white));

        QString text;
        QwtText qwtText;
        // 遍历所有曲线点
        for (int i = 0; i < m_curve->dataSize(); ++i) {
            QPointF sample = m_curve->sample(i);
            if (sample.x() == closestPoint.x() && sample.y() == closestPoint.y()){
                text = QString("Time: %1").arg(timeslist[i]);

                qwtText.setText(text);
                qwtText.setBackgroundBrush(QBrush(Qt::white));
            }
        }
        qDebug()<<closestPoint;
        return qwtText;
    }
    else
    {
        return QwtText(); // 如果没有找到接近的点，返回空文本
    }
}

void  HoverPicker::widgetMousePressEvent(QMouseEvent* e)
{
    QTime currentTime = QTime::currentTime();
    bool isDoubleClick = false;
    m_lastClickTime = currentTime;
    if (e->button() == Qt::LeftButton && QApplication::keyboardModifiers() & Qt::ControlModifier)
    {

        qDebug() << "m_lastClickTime:" << m_lastClickTime<<m_lastClickTime.msecsTo(currentTime)<<QApplication::doubleClickInterval();
        if (m_lastClickTime.isValid() && m_lastClickTime.msecsTo(currentTime) < 500)
        {
            isDoubleClick = true;
        }

        if (isDoubleClick)
        {
            // QPointF plotPos =invTransform(canvas()->mapFromGlobal(e->globalPos()));

            // double minDist = 0.1;
            // QPointF closestPoint;

            // for (int i = 0; i < m_curve->dataSize(); ++i)
            // {
            //     QPointF sample = m_curve->sample(i);
            //     double dist = QLineF(plotPos, sample).length();
            //     if (dist < minDist)
            //     {
            //         minDist = dist;
            //         closestPoint = sample;
            //     }
            // }

            // qDebug() << "Double Clicked Point:" << closestPoint;
            // m_lastClickedPoint = closestPoint;




            QPointF mouse_pos =invTransform(canvas()->mapFromGlobal(e->globalPos()));
            double minDist = 10;
            QPointF closestPoint;

            if (first_node != QPointF(3.14, 3.14) && second_node != QPointF(3.14, 3.14)) {
                first_node = QPointF(3.14, 3.14);
                second_node = QPointF(3.14, 3.14);
            }

            for (int i = 0; i < m_curve->dataSize(); ++i) {
                QPointF sample = m_curve->sample(i);
                double dist = QLineF(mouse_pos, sample).length();
                if (dist < minDist){

                     closestPoint = sample;
                } else {
                    if (first_node == QPointF(3.14, 3.14)) {
                        break;
                    } else {
                        second_node = QPointF(3.14, 3.14);
                    }
                }
            }

            bool first_click = (first_node == QPointF(3.14, 3.14));
            qDebug() << first_click;
            if (first_click) {
                for (int i = 0; i < m_curve->dataSize(); ++i) {
                    QPointF sample = m_curve->sample(i);
                    double dist = QLineF(mouse_pos, sample).length();
                    if (dist < minDist){
                        first_node = sample;
                        first_click=false;
                        qDebug() << first_node.x() << ", " << first_node.y();
                        break;
                    }
                }
            } else {
                for (int i = 0; i < m_curve->dataSize(); ++i) {
                    QPointF sample = m_curve->sample(i);
                    double dist = QLineF(mouse_pos, sample).length();
                    qDebug() << dist<< "," << minDist;
                    if (dist < minDist){
                        second_node = sample;
                        first_click=false;
                        qDebug() << second_node.x() << ", " << second_node.y();
                        break;
                    }
                }
            }

            if (first_node != QPointF(3.14, 3.14) && first_node == second_node) {
                second_node = QPointF(3.14, 3.14);
            }
            if (first_node != QPointF(3.14, 3.14) && second_node != QPointF(3.14, 3.14)){
                QString text1;
                QString text2;
                QwtText qwtText;
                for (int i = 0; i < m_curve->dataSize(); ++i) {
                    QPointF sample = m_curve->sample(i);
                    if (sample.x() == first_node.x() && sample.y() == first_node.y()){
                        text1 = timeslist[i];

                        qwtText.setText(text1);
                        qwtText.setBackgroundBrush(QBrush(Qt::white));
                    }
                    if (sample.x() == second_node.x() && sample.y() == second_node.y()){
                        text2 = timeslist[i];

                        qwtText.setText(text2);
                        qwtText.setBackgroundBrush(QBrush(Qt::white));
                    }
                }

                QTime time1 = QTime::fromString(text1, "hh:mm:ss");
                QTime time2 = QTime::fromString(text2, "hh:mm:ss");

                // 计算时间差异（以秒为单位）
                int seconds1 = QTime(0, 0, 0).secsTo(time1);  // 将 time1 转换为秒
                int seconds2 = QTime(0, 0, 0).secsTo(time2);  // 将 time2 转换为秒
                if ((seconds1-seconds2)>0){
                    QString temp;
                    temp=text1;
                    text1=text2;
                    text2=temp;
                }
                QString infoText = QString("Time Start: %1\nTime End: %2")
                .arg(text1)
                .arg(text2);
                m_infoLabel->setText(infoText);
                m_infoLabel->adjustSize();
                m_infoLabel->show();
                emit startandendtime(text1,text2);
            }

            if (first_node != QPointF(3.14, 3.14) && second_node == QPointF(3.14, 3.14)){
                QString text1;
                QString text2;
                QwtText qwtText;
                for (int i = 0; i < m_curve->dataSize(); ++i) {
                    QPointF sample = m_curve->sample(i);
                    if (sample.x() == first_node.x() && sample.y() == first_node.y()){
                        text1 = timeslist[i];

                        qwtText.setText(text1);
                        qwtText.setBackgroundBrush(QBrush(Qt::white));
                    }

                }
                QString infoText = QString("Time Start: %1")
                .arg(text1);
                m_infoLabel->setText(infoText);
                m_infoLabel->adjustSize();
                m_infoLabel->show();
            }

            if (first_node == QPointF(3.14, 3.14)){
                QString infoText = QString("Points Information")
                .arg(first_node.x())
                    .arg(first_node.y())
                    .arg(second_node.y())
                    .arg(second_node.y());
                m_infoLabel->setText(infoText);
                m_infoLabel->adjustSize();
                m_infoLabel->show();
            }
            qDebug() << first_node.x()<< first_node.x() << ", " << first_node.y() << " | " << second_node.x() << ", " << second_node.y();

        }


    }
    // 调用基类实现以确保基本的鼠标事件处理
    QwtPlotPicker::widgetMousePressEvent(e);
}


namespace
{
class DistancePicker : public QwtPlotPicker
{
public:
    DistancePicker( QWidget* canvas )
        : QwtPlotPicker( canvas )
    {
        setTrackerMode( QwtPicker::ActiveOnly );
        setStateMachine( new QwtPickerDragLineMachine() );
        setRubberBand( QwtPlotPicker::PolygonRubberBand );
    }

    virtual QwtText trackerTextF( const QPointF& pos ) const QWT_OVERRIDE
    {
        QwtText text;

        const QPolygon points = selection();
        if ( !points.isEmpty() )
        {
            QString num;
            num.setNum( QLineF( pos, invTransform( points[0] ) ).length() );

            QColor bg( Qt::white );
            bg.setAlpha( 200 );

            text.setBackgroundBrush( QBrush( bg ) );
            text.setText( num );
        }
        return text;
    }
};
}

Plot::Plot( QWidget* parent )
    : QwtPlot( parent )

{
    canvas()->setStyleSheet(
        "border: 2px solid Black;"
        "border-radius: 15px;"
        "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1,"
        "stop: 0 LemonChiffon, stop: 1 PaleGoldenrod );"
        );

    // attach curve
    m_curve = new QwtPlotCurve( "Scattered Points" );
    // m_curve->setPen(QColor( "Purple" ));
    // m_curve->setPen( QPen(Qt::blue, 2));

    m_curve->setRenderThreadCount( 0 ); // 0: use QThread::idealThreadCount()

    setSymbol( NULL );


    (void )new QwtPlotPanner( canvas() );

    QwtPlotMagnifier* magnifier = new QwtPlotMagnifier( canvas() );
    magnifier->setMouseButton( Qt::NoButton );

    DistancePicker* picker = new DistancePicker( canvas() );
    picker->setMousePattern( QwtPlotPicker::MouseSelect1, Qt::RightButton );
    picker->setRubberBandPen( QPen( Qt::blue ) );



    // m_curve->attach( this );
}


void Plot::setSymbol( QwtSymbol* symbol )
{
    m_curve->setSymbol( symbol );

    if ( symbol == NULL )
    {
        m_curve->setStyle( QwtPlotCurve::Dots );
    }
}

void Plot::setSamples( QwtPlotCurve* curve,const QVector< QPointF >& samples )
{   qDebug() <<"QwtText" << curve->dataSize();
    curve->setPaintAttribute(
        QwtPlotCurve::ImageBuffer, samples.size() > 1000 );

    // curve->setSamples( samples );
    // 创建显示点信息的标签，并将其放置在右上角
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("background-color: white; border: 1px solid black;");
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // m_infoLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_infoLabel->move(width() - m_infoLabel->width() + 600, 10);

    connect(this, &Plot::resizeEvent, this, [this]() {
        m_infoLabel->move(width() - m_infoLabel->width() + 600, 10);
    });
    qDebug() <<"QwtText1" << curve->dataSize();
    QStringList  nonlist;
    HoverPicker* hoverPicker = new HoverPicker(canvas(), curve,m_infoLabel, nonlist);
    hoverPicker->setRubberBandPen(QPen(Qt::blue));
    // 右键测距

}

void Plot::setSamplesLIN( QwtPlotCurve* curve,const QVector< QPointF >& samples,QStringList & timelist )
{   qDebug() <<"QwtText" << curve->dataSize();
    curve->setPaintAttribute(
        QwtPlotCurve::ImageBuffer, samples.size() > 1000 );

    // curve->setSamples( samples );
    // 创建显示点信息的标签，并将其放置在右上角
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("background-color: white; border: 1px solid black;");
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // m_infoLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_infoLabel->move(width() - m_infoLabel->width() + 600, 10);

    connect(this, &Plot::resizeEvent, this, [this]() {
        m_infoLabel->move(width() - m_infoLabel->width() + 600, 10);
    });
    timeslist=timelist;
    qDebug() <<"QwtText1" << curve->dataSize();
    QStringList  nonlist;
    HoverPicker* hoverPicker = new HoverPicker(canvas(), curve,m_infoLabel, timeslist);
    hoverPicker->setRubberBandPen(QPen(Qt::blue));
    connect(hoverPicker,&HoverPicker::startandendtime,this, &Plot::setSamplesLINtime);
    // 右键测距

}

void Plot::setSamplesLINtime(QString starttime,QString endtime){
    emit LINtime(starttime,endtime);
}

void Plot::setSamplesTipper( QwtPlotCurve* curve)
{   qDebug() <<"QwtText" << curve->dataSize();

    // curve->setSamples( samples );
    // 创建显示点信息的标签，并将其放置在右上角
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("background-color: white; border: 1px solid black;");
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // m_infoLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_infoLabel->move(width() - m_infoLabel->width() + 600, 10);

    connect(this, &Plot::resizeEvent, this, [this]() {
        m_infoLabel->move(width() - m_infoLabel->width() + 600, 10);
    });
    qDebug() <<"QwtText1" << curve->dataSize();
    QStringList  nonlist;
    HoverPicker* hoverPicker = new HoverPicker(canvas(), curve,m_infoLabel, nonlist);
    hoverPicker->setRubberBandPen(QPen(Qt::blue));

    // 右键测距

}

#include "moc_Plot.cpp"
