#include "copyworker.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

CopyWorker::CopyWorker(const QString& sourcePath, const QString& destinationPath, QObject* parent)
    : QObject(parent), m_sourcePath(sourcePath), m_destinationPath(destinationPath) {}

void CopyWorker::startCopy() {
    QFileInfo sourceInfo(m_sourcePath);
    bool success = false;

    if (sourceInfo.isDir()) {
        success = copyDirectoryContents(m_sourcePath, m_destinationPath);
    } else if (sourceInfo.isFile()) {
        QString destFilePath = m_destinationPath + "/" + sourceInfo.fileName();
        success = copyFile(m_sourcePath, destFilePath);
    }

    emit copyFinished(success);
}

bool CopyWorker::copyDirectoryContents(const QString &sourceDirPath, const QString &destDirPath) {
    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
        qWarning() << "Source directory does not exist:" << sourceDirPath;
        return false;
    }

    QDir destDir(destDirPath);
    if (!destDir.exists()) {
        if (!destDir.mkpath(destDirPath)) {
            qWarning() << "Failed to create destination directory:" << destDirPath;
            return false;
        }
    }

    QStringList entries = sourceDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    int totalEntries = entries.size();
    int processedEntries = 0;

    // 复制内容
    foreach (QString entry, entries) {
        QString sourceEntryPath = sourceDir.absoluteFilePath(entry);
        QString destEntryPath = destDir.absoluteFilePath(entry);

        QFileInfo entryInfo(sourceEntryPath);
        if (entryInfo.isDir()) {
            if (!copyDirectoryContents(sourceEntryPath, destEntryPath)) {
                return false;
            }
        } else {
            QString destfileabs = destEntryPath;
            QFile file(destfileabs);

            if (file.exists()) {
                file.remove();
            }
            if (!QFile::copy(sourceEntryPath, destEntryPath)) {
                qWarning() << "Failed to copy file:" << sourceEntryPath << "to" << destEntryPath;
                return false;
            }
        }

        // Update progress
        processedEntries++;
        int progress = (processedEntries * 100) / totalEntries;
        emit progressUpdated(progress);
    }

    return true;
}

bool CopyWorker::copyFile(const QString &sourceFilePath, const QString &destFilePath) {
    QFileInfo sourceFileInfo(sourceFilePath);
    if (!sourceFileInfo.exists()) {
        qWarning() << "Source file does not exist:" << sourceFilePath;
        return false;
    }
    QString destfileabs = destFilePath;
    QFile file(destfileabs);

    if (file.exists()) {
        if (file.remove()) {
            qDebug() << "File removed successfully.";
        } else {
            qDebug() << "Failed to remove the file.";
        }
    }
    QDir destDir = QFileInfo(destFilePath).absoluteDir();
    if (!destDir.exists()) {
        if (!destDir.mkpath(destDir.absolutePath())) {
            qWarning() << "Failed to create destination directory:" << destDir.absolutePath();
            return false;
        }
    }

    if (!QFile::copy(sourceFilePath, destFilePath)) {
        qWarning() << "Failed to copy file:" << sourceFilePath << "to" << destFilePath;
        return false;
    }

    // File copy progress (in this case, it's always 100% for individual files)
    emit progressUpdated(100);

    return true;
}
