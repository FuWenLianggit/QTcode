#include "Inverseprocess.h"

#include <QProcessEnvironment>
#include <QDebug>
#include <QCoreApplication>
#include "MainWindow.h"

Inverseprocess::Inverseprocess(QObject *parent) : QThread(parent), process(new QProcess(this)),mainwindow(static_cast<MainWindow*>(parent))
{
}

// 倾子计算在子线程中进行 主线程中将使用进度条阻塞
void Inverseprocess::run()
{
    qDebug() << "mainwindow->inverseprocesslineworking"<<  mainwindow->inverseprocesslineworking;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();


    try {
        qDebug() << "Dialog1"<<  mainwindow->inverseprocesslineworking;
        throw std::runtime_error("Sample error message");
    } catch (const std::exception &e) {
        qDebug() << "Caught exception:" << e.what(); // 输出错误信息
    }


    QString exePath = QDir::currentPath() + "/bins/zteminv.exe";
    QString program = exePath;

    qDebug() << "Program path: " << program;

    QStringList arguments(mainwindow->inverseprocessparameters);

    QDir dir(mainwindow->inverseprocesslineworking);

    QString work = dir.path();
    qDebug() << "setWorkingDirectory"<<  work;

    process->setWorkingDirectory(work);
    process->start(program, arguments);

    if (!process->waitForStarted()) {
        emit finished("", "Failed to start process: " + process->errorString(),mainwindow->inverseprocesslineworking);
        return;
    }

    if (!process->waitForFinished(-1)) {
        emit finished("", "Process failed to finish: " + process->errorString(),mainwindow->inverseprocesslineworking);
        return;
    }

    QString output = process->readAllStandardOutput().data();
    QString errorOutput = process->readAllStandardError().data();

    emit finished(output, errorOutput,mainwindow->inverseprocesslineworking);
}
