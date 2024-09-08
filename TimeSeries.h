#ifndef TIMSERIES_H
#define TIMSERIES_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QString>

class TimeSeries  {


public:
    explicit TimeSeries(const QString& TsNamein);
    QVector<QDateTime> getTimeData() const;
    QVector<QVector<double>> getData() const;
public slots:
    void readfile();

signals:
    void returndata(QVector<QDateTime> timedata, QVector<QVector<double>> data);

private:


    QString TsName;
    QDateTime startTime;
    int sampleFreq;
    QVector<QVector<double>> data;
};

#endif // TIMSERIES_H
