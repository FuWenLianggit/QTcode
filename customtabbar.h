#ifndef CUSTOMTABBAR_H
#define CUSTOMTABBAR_H

#include <QTabBar>
#include <QPainter>
#include <QIcon>
#include <QPixmap>

class CustomTabBar : public QTabBar {
    Q_OBJECT

public:
    CustomTabBar(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QRect getCloseButtonRect(const QRect &tabRect);
};

#endif // CUSTOMTABBAR_H
