#include "CustomFilterProxyModel.h"
#include <QFileSystemModel>
#include <QFileInfo>
#include <QDebug>  // 使用 QDebug 进行调试输出

CustomFilterProxyModel::CustomFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void CustomFilterProxyModel::setFolderNameFilter(const QString &workAreaName)
{
    // 设置过滤器的名称
    m_folderNameFilter = workAreaName;
    // 输出调试信息，确认名称已设置
    qDebug() << "Filter name set to:" << m_folderNameFilter;
    // 使过滤器重新应用
    invalidateFilter();
}


bool CustomFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent) const
{

    // 获取源模型的索引
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    // 将源模型转换为 QFileSystemModel 类型
    QFileSystemModel *fileSystemModel = qobject_cast<QFileSystemModel *>(sourceModel());


    if (!fileSystemModel) {
        qDebug() << "sourceModel() is not a QFileSystemModel";
        return false;
    }

    // 获取文件信息
    QFileInfo fileInfo = fileSystemModel->fileInfo(index);
    qDebug() << "Checking file:" << fileInfo.filePath() <<"m_folderNameFilter"<<m_folderNameFilter;
    if (m_folderNameFilter.isEmpty()){
        qDebug() << "isEmpty:" << fileInfo.filePath() <<"m_folderNameFilter"<<m_folderNameFilter;
        return false;
    }

    // 只显示名称匹配的文件夹
    if (fileInfo.fileName() == m_folderNameFilter || fileInfo.filePath().contains(m_folderNameFilter) ) {
        // if (fileInfo.fileName().endsWith("Folder1")) {
            // 输出过滤器的名称，确保其值正确
        qDebug() << "isDir:" << fileInfo.filePath() <<"m_folderNameFilter"<<m_folderNameFilter;
        if (m_folderNameFilter.isEmpty()){
            return false;
        }else{
            bool isAccepted = ((fileInfo.fileName() == m_folderNameFilter));
            qDebug() << "Directory accepted:" << isAccepted;
            return true;
        }
        // }
    }

    // 对于文件，不进行过滤
    qDebug() << "Not a directory, filtering out.";
    return false;


}
