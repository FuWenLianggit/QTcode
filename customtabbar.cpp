#include "customtabbar.h"
#include <QMouseEvent>
#include <QTabWidget>
#include <iostream> // 用于 std::cout
CustomTabBar::CustomTabBar(QWidget *parent) : QTabBar(parent) {}

void CustomTabBar::paintEvent(QPaintEvent *event) {
    QTabBar::paintEvent(event);
    QPainter painter(this);
    for (int index = 0; index < count(); ++index) {
        QRect tabRect = this->tabRect(index);
        QRect closeButtonRect = getCloseButtonRect(tabRect);
        QIcon icon(":/images/close_icon.png");  // 使用自定义的关闭按钮图标
        QPixmap iconPixmap = icon.pixmap(closeButtonRect.size());
        painter.drawPixmap(closeButtonRect, iconPixmap);
    }
}

QRect CustomTabBar::getCloseButtonRect(const QRect &tabRect) {
    int buttonWidth = 16;
    int buttonHeight = 16;
    int buttonX = tabRect.right() - buttonWidth - 5;  // 从右侧边缘向左偏移5像素
    int buttonY = tabRect.top() + (tabRect.height() - buttonHeight) / 2;  // 垂直居中
    return QRect(buttonX, buttonY, buttonWidth, buttonHeight);
}

void CustomTabBar::mousePressEvent(QMouseEvent *event) {
    // 获取鼠标点击位置所在的标签页索引
    int tabIndex = tabAt(event->pos());

    // 检查是否点击在有效的标签页上
    if (tabIndex != -1) {
        QRect tabRect = this->tabRect(tabIndex);

        QRect closeButtonRect = getCloseButtonRect(tabRect);

        if (closeButtonRect.contains(event->pos())) {


            QTabWidget *tabWidget = qobject_cast<QTabWidget *>(parentWidget());
            if (tabWidget) {

                tabWidget->removeTab(tabIndex);
                if (tabWidget->count()==0){
                    tabWidget->clear();
                }
            }
            return;
        }
    }

    // 调用父类的 mousePressEvent 事件处理
    QTabBar::mousePressEvent(event);
}

