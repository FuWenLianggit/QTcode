#include "TimeSeries.h"
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <stdexcept>

TimeSeries::TimeSeries(const QString& TsNamein, QObject *parent)
    : QObject(parent), TsName(TsNamein){
    // readfile();

}
void removeZeroes(QVector<QVector<double>>& data) {
    // Iterate over each row in data
    for (int i = 0; i < data.size(); ++i) {
        QVector<double> row = data[i];
        // Remove elements that are zero
        row.erase(std::remove(row.begin(), row.end(), 0.0), row.end());
        // Update the row in the original data
        data[i] = row;
    }

    // Remove rows that have become empty after removing zeroes
    data.erase(std::remove_if(data.begin(), data.end(), [](const QVector<double>& row) {
                   return row.isEmpty();
               }), data.end());
}
void TimeSeries::readfilepart(const QDateTime& actualStart, const QDateTime& actualEnd) {
    QFile file(TsName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Unable to open file");
    }

    QTextStream in(&file);
    QStringList lines;

    // 读取并修剪空白字符，去除空行
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }

    // 解析开始时间
    QString startTimeString = lines[0].mid(5).trimmed();
    startTime = QDateTime::fromString(startTimeString, "yyyy/MM/dd HH:mm:ss");

    // 解析采样频率
    QString freqString = lines[3].split(":")[1].trimmed();
    sampleFreq = freqString.toInt();

    int numDataPoints = (lines.size() / (sampleFreq + 5)) * sampleFreq;
    data.resize(numDataPoints, QVector<double>(5, 0.0));
    qDebug()<< data.size();
    int idx = 0;
    int segIdx = 0;


    // 根据时间范围读取数据
    timeAxis = TimeData();
    int startIdx = -1, endIdx = -1, startIdt=-1  ,endIdt = -1;

    // 查找时间范围对应的索引
    for (int i = 0; i < timeAxis.size(); ++i) {
        if (timeAxis[i] >= actualStart && startIdx == -1) {
            startIdx = i+(i/sampleFreq)*5;
            startIdt=i+(i/sampleFreq)*5;
        }
        if (timeAxis[i] <= actualEnd) {
            endIdx = i;
            endIdt=i;
        }
    }
    qDebug() << "startIdx" << startIdx << "endIdx" << endIdx;
    qDebug()<< startIdx << lines[startIdx];
    while (startIdx < endIdx) {
        if (lines[startIdx].startsWith("Time:")) {
            // qDebug()<< startIdx << lines[startIdx];
            startIdx += 5;
        }

        for (int i = 0; i < sampleFreq; ++i) {
            QStringList values = lines[startIdx + i].split(' ');
            QVector<double> row;
            for (const QString& val : values) {
                row.append(val.toDouble());
            }
            data[segIdx * sampleFreq + i] = row;

        }
        startIdx += sampleFreq;
        segIdx++;
    }
    removeZeroes(data);
    QVector<QDateTime> timetmp;
    timetmp = timeAxis;
    int id=0;
    qDebug() << timeAxis.size() << data.size();

    while(startIdt < endIdt){
        timeAxis.resize(data.size());

        timeAxis[id] = timetmp[startIdt];
        startIdt++;
    }
    qDebug() <<  timeAxis.size() << data.size();

    // for (int i; i<data.size();i++){
    //     qDebug() << i << data[i];
    //     if(data[i][4]<2000){
    //         qDebug() << i << data[i];
    //     }

    // }

    qDebug() << "data" << data[0];
    file.close();

    // emit returndata(getTimeData(), getData());
}


void TimeSeries::readfile() {
    QFile file(TsName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Unable to open file");
    }

    QTextStream in(&file);
    QStringList lines;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // 读取并修剪空白字符
        if (!line.isEmpty()) {
            lines.append(line); // 只添加非空行
        }
    }
    qDebug() << "lines" << lines[0];
    // Parsing start time
    QString startTimeString = lines[0].mid(5).trimmed();
    startTime = QDateTime::fromString(startTimeString, "yyyy/MM/dd HH:mm:ss");
    qDebug() << "startTime" << startTime;
    // Parsing sample frequency
    QString freqString = lines[3].split(":")[1].trimmed();
    sampleFreq = freqString.toInt();
    qDebug() << "sampleFreq" << sampleFreq;
    int numDataPoints = (lines.size() / (sampleFreq + 5)) * sampleFreq;
    data.resize(numDataPoints, QVector<double>(5, 0.0));

    int idx = 0;
    int segIdx = 0;

    while (idx < lines.size()) {
        if (lines[idx].startsWith("Time:")) {
            idx += 5;
        }

        for (int i = 0; i < sampleFreq; ++i) {
            QStringList values = lines[idx + i].split(' ');
            QVector<double> row;
            for (const QString& val : values) {
                row.append(val.toDouble());
            }
            data[segIdx * sampleFreq + i] = row;
        }
        idx += sampleFreq;
        segIdx++;
    }
    removeZeroes(data);
    qDebug() << "data" << data[0];
    file.close();

}

QVector<QDateTime> TimeSeries::getTimeData() const {
    return timeAxis;
}

QVector<QDateTime> TimeSeries::TimeData() const {
    int numSamples = data.size();
    QVector<QDateTime> timeAxis(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        timeAxis[i] = startTime.addSecs(i / sampleFreq);
    }
    return timeAxis;
}


QVector<QVector<double>> TimeSeries::getData() const {
    return data;
}


