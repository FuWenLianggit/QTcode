#ifndef CLOSEBUTTONFILTER_H
#define CLOSEBUTTONFILTER_H

#include <QEvent>
#include <QObject>
#include <QTabWidget>
#include <QMouseEvent>
#include <QTabBar>
#include <iostream> // 用于 std::cout
class CloseButtonFilter : public QObject {
    Q_OBJECT

public:
    CloseButtonFilter(QTabWidget *tabWidget, QObject *parent = nullptr)
        : QObject(parent), tabWidget(tabWidget) {}

protected:

    bool eventFilter(QObject *obj, QEvent *event) override {

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            std::cout << "Button: " << mouseEvent->pos().x() <<mouseEvent->pos().y()<<std::endl;
            QTabBar *tabBar = qobject_cast<QTabBar *>(obj);  // 使用 qobject_cast 进行类型转换

            if (obj == tabWidget->tabBar() && mouseEvent->button() == Qt::LeftButton) {
                QPoint clickPos = mouseEvent->pos();
                clickPos.setX(clickPos.x()-10);
                int tabIndex = tabBar->tabAt(mouseEvent->pos());
                // tabWidget->parentWidget()->findChild<QTabWidget *>()->removeTab(tabIndex);
                // return true;
                if (tabIndex != -1) {


                    QRect tabRect = tabBar->tabRect(tabIndex);
                    int buttonWidth = 16;
                    int buttonHeight = 16;
                    int buttonX = tabRect.right() - buttonWidth -5;  // 从右侧边缘向左偏移5像素
                    int buttonY = tabRect.top() + (tabRect.height() - buttonHeight) / 2;  // 垂直居中
                    QRect closeButtonRect = QRect(buttonX, buttonY, buttonWidth, buttonHeight);
                    std::cout << "Button: " << clickPos.x() <<clickPos.y()<<buttonX<<buttonY<< std::endl;
                    if (closeButtonRect.contains(clickPos)) {
                        tabWidget->parentWidget()->findChild<QTabWidget *>()->removeTab(tabIndex);
                        return true;
                    }

                }
            }
        }

        return QObject::eventFilter(obj, event);
    }

private:
    QTabWidget *tabWidget;

};

#endif // CLOSEBUTTONFILTER_H
