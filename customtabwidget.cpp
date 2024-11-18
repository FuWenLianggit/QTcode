#include "customtabwidget.h"
#include "customtabbar.h"

// TabWidget中所支持的功能无法实现项目需要 重新定义TabWidget的settabbar函数

CustomTabWidget::CustomTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    // 在构造函数内部创建 CustomTabBar 对象
    customTabBar = new CustomTabBar(this);

    // 设置自定义的 tab bar
    setTabBar(customTabBar);
}
