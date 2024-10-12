#ifndef TIMERANGEDIALOG_H
#define TIMERANGEDIALOG_H

#include <QDialog>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>

class TimeRangeDialog : public QDialog {
    Q_OBJECT

public:
    TimeRangeDialog(QDateTime start, QDateTime end, QWidget *parent = nullptr);

    // 获取用户选择的时间范围
    QPair<QTime, QTime> getSelectedTimeRange() const;

private:
    QDateTime startTime;
    QDateTime endTime;
    QDateTimeEdit *startTimeEdit;
    QDateTimeEdit *endTimeEdit;
    QLabel *startTimeLabel;
    QLabel *endTimeLabel;
    QLabel *instructionLabel;
};

#endif // TIMERANGEDIALOG_H
