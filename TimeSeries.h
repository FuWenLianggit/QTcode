#ifndef TIMSERIES_H
#define TIMSERIES_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QString>

class TimeSeries : public QObject {
    Q_OBJECT

public:
    explicit TimeSeries(const QString& TsName, QObject *parent = nullptr);

    QVector<QDateTime> getTimeData() const;
    QVector<QDateTime> TimeData() const;
    QVector<QVector<double>> getData() const;
public slots:
    void readfile();
    void readfilepart(const QDateTime& actualStart, const QDateTime& actualEnd);

signals:
    void returndata(QVector<QDateTime> timedata, QVector<QVector<double>> data);

private:

    QVector<QDateTime> timeAxis;
    QString TsName;
    QDateTime startTime;
    int sampleFreq;
    QVector<QVector<double>> data;
};

#endif // TIMSERIES_H
