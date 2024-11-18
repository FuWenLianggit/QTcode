#ifndef INVERSEPROCESS_H
#define INVERSEPROCESS_H

#include <QThread>
#include <QProcess>
#include <QString>


class MainWindow;

class Inverseprocess : public QThread
{
    Q_OBJECT

public:
    explicit Inverseprocess(QObject *parent = nullptr);

signals:
    void finished(const QString &output, const QString &error ,const QString &Path);

protected:
    void run() override;

private:
    QProcess *process;
    MainWindow *mainwindow;
signals:
    void workFinished(const QString &result);  // 发射任务完成的信号
};


#endif // INVERSEPROCESS_H
