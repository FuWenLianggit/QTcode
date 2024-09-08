#include "customtabwidget.h"
#include "customtabbar.h"

CustomTabWidget::CustomTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    // 在构造函数内部创建 CustomTabBar 对象
    customTabBar = new CustomTabBar(this);

    // 设置自定义的 tab bar
    setTabBar(customTabBar);
}
