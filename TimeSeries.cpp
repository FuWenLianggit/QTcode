#include "TimeSeries.h"
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <stdexcept>

TimeSeries::TimeSeries(const QString& TsNamein ){
    TsName = TsNamein;
    readfile();

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


    // QVector<QDateTime> timeAxis = this->getTimeData();
    // QVector<QVector<double>> datas = this->getData();
    // emit returndata(timeAxis,datas);
}

QVector<QDateTime> TimeSeries::getTimeData() const {
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


