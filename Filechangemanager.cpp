#include "FileChangeManager.h"

FileChangeManager::FileChangeManager(const QString& jsonFilePath)
    : m_jsonFilePath(jsonFilePath)
{
}

void FileChangeManager::initializeFileInfo(const QFileInfoList& fileAndFolderList)
{
    QJsonArray initialFileInfoArray;
    foreach (const QFileInfo &fileInfo, fileAndFolderList) {
        QJsonObject fileObject;
        fileObject["path"] = fileInfo.absoluteFilePath();
        fileObject["size"] = fileInfo.size();
        fileObject["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
        initialFileInfoArray.append(fileObject);
    }

    QJsonObject rootObject;
    rootObject["initialFileInfo"] = initialFileInfoArray;

    QJsonDocument jsonDoc(rootObject);
    saveJson(jsonDoc);
}

void FileChangeManager::checkAndRecordChanges(const QFileInfoList& fileAndFolderList)
{
    QJsonDocument jsonDoc = loadJson();
    QJsonObject rootObject = jsonDoc.object();
    QJsonArray initialFileInfoArray = rootObject["initialFileInfo"].toArray();
    QMap<QString, QJsonObject> initialFileMap;

    foreach (const QJsonValue &value, initialFileInfoArray) {
        QJsonObject fileObject = value.toObject();
        initialFileMap[fileObject["path"].toString()] = fileObject;
    }

    QJsonArray changesArray;
    foreach (const QFileInfo &fileInfo, fileAndFolderList) {
        QString path = fileInfo.absoluteFilePath();
        if (initialFileMap.contains(path)) {
            QJsonObject initialFileObject = initialFileMap[path];
            if (initialFileObject["size"].toVariant().toLongLong() != fileInfo.size() ||
                QDateTime::fromString(initialFileObject["lastModified"].toString(), Qt::ISODate) != fileInfo.lastModified()) {

                QJsonObject changeObject;
                changeObject["path"] = path;
                changeObject["oldSize"] = initialFileObject["size"];
                changeObject["newSize"] = fileInfo.size();
                changeObject["oldLastModified"] = initialFileObject["lastModified"];
                changeObject["newLastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
                changesArray.append(changeObject);

                // 更新初始信息
                initialFileMap[path]["size"] = fileInfo.size();
                initialFileMap[path]["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
            }
        } else {
            // 新增的文件
            QJsonObject newFileObject;
            newFileObject["path"] = path;
            newFileObject["size"] = fileInfo.size();
            newFileObject["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
            changesArray.append(newFileObject);

            initialFileMap[path] = newFileObject;
        }
    }

    // 更新 JSON 文件
    rootObject["changes"] = changesArray;
    QJsonArray updatedInitialFileInfoArray;
    foreach (const QJsonObject &fileObject, initialFileMap) {
        updatedInitialFileInfoArray.append(fileObject);
    }
    rootObject["initialFileInfo"] = updatedInitialFileInfoArray;

    jsonDoc.setObject(rootObject);
    saveJson(jsonDoc);
}

QJsonDocument FileChangeManager::loadJson() const
{
    QFile jsonFile(m_jsonFilePath);
    if (jsonFile.open(QIODevice::ReadOnly)) {
        return QJsonDocument::fromJson(jsonFile.readAll());
    }
    return QJsonDocument();
}

void FileChangeManager::saveJson(const QJsonDocument& jsonDoc) const
{
    QFile jsonFile(m_jsonFilePath);
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(jsonDoc.toJson());
        jsonFile.close();
    }
}
