#include "TimeRangeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
// 绘制原始数据前打开时间选择界面 返回时间到主界面
TimeRangeDialog::TimeRangeDialog(QDateTime start, QDateTime end, QWidget *parent)
    : QDialog(parent), startTime(start), endTime(end) {

    // 提示文字，显示起止时间范围
    instructionLabel = new QLabel(tr("请选择需要展示的时间范围 (%1 到 %2):")
                                      .arg(startTime.toString("HH:mm:ss"), endTime.toString("HH:mm:ss")), this);
    instructionLabel->setAlignment(Qt::AlignCenter); // 文本居中

    // 时间选择标签和控件
    startTimeLabel = new QLabel("开始时间:", this);
    startTimeEdit = new QDateTimeEdit(startTime.time(), this);
    startTimeEdit->setDisplayFormat("HH:mm:ss");

    endTimeLabel = new QLabel("结束时间:", this);
    endTimeEdit = new QDateTimeEdit(endTime.time(), this);
    endTimeEdit->setDisplayFormat("HH:mm:ss");

    // 设置布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *startTimeLayout = new QHBoxLayout();
    QHBoxLayout *endTimeLayout = new QHBoxLayout();

    // 将控件和标签添加到布局
    startTimeLayout->addWidget(startTimeLabel);
    startTimeLayout->addWidget(startTimeEdit);
    endTimeLayout->addWidget(endTimeLabel);
    endTimeLayout->addWidget(endTimeEdit);

    // 将元素添加到主布局
    mainLayout->addWidget(instructionLabel);
    mainLayout->addLayout(startTimeLayout);
    mainLayout->addLayout(endTimeLayout);

    // 确认按钮
    QPushButton *confirmButton = new QPushButton("确认", this);
    confirmButton->setStyleSheet("QPushButton { background-color: #4caf50; color: white; border-radius: 5px; padding: 5px; "
                                 "font-weight: bold; font-size: 14px; min-height: 30px; }");
    mainLayout->addWidget(confirmButton);
    mainLayout->setAlignment(confirmButton, Qt::AlignRight); // 按钮右对齐

    // 连接按钮的点击事件
    connect(confirmButton, &QPushButton::clicked, this, &TimeRangeDialog::accept);

    // 设置弹窗的样式
    setStyleSheet("QLabel { font-size: 12px; color: #333333; }"
                  "QDateTimeEdit { background-color: #ffffff; border: 1px solid #cccccc; border-radius: 5px; padding: 5px; "
                  "min-width: 150px; font-size: 12px; }"
                  "QDialog { border: 2px solid #2a2a2a; border-radius: 10px; background-color: #f0f0f0; padding: 20px; }");

    setWindowTitle("选择时间范围");
    setFixedSize(350, 220); // 设置固定大小，保持布局一致
}

QPair<QTime, QTime> TimeRangeDialog::getSelectedTimeRange() const {
    return {startTimeEdit->time(), endTimeEdit->time()};
}
