#ifndef FILECHANGEMANAGER_H
#define FILECHANGEMANAGER_H

#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QFile>
#include <QString>
#include <QMap>

class FileChangeManager
{
public:
    explicit FileChangeManager(const QString& jsonFilePath);

    void initializeFileInfo(const QFileInfoList& fileAndFolderList);
    void checkAndRecordChanges(const QFileInfoList& fileAndFolderList);
    QJsonDocument loadJson() const;
    void saveJson(const QJsonDocument& jsonDoc) const;

private:
    QString m_jsonFilePath;
};

#endif // FILECHANGEMANAGER_H
