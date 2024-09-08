// copyworker.h
#ifndef COPYWORKER_H
#define COPYWORKER_H

#include <QObject>
#include <QString>

class CopyWorker : public QObject {
    Q_OBJECT

public:
    CopyWorker(const QString &sourcePath, const QString &destinationPath, QObject *parent = nullptr);

public slots:
    void startCopy();

signals:
    void copyFinished(bool success);
    void progressUpdated(int percentage);

private:
    bool copyDirectoryContents(const QString &sourceDirPath, const QString &destDirPath);
    bool copyFile(const QString &sourceFilePath, const QString &destFilePath);

    QString m_sourcePath;
    QString m_destinationPath;
};

#endif // COPYWORKER_H
