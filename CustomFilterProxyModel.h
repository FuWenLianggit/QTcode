#ifndef CUSTOMFILTERPROXYMODEL_H
#define CUSTOMFILTERPROXYMODEL_H
#include <QFileInfoList>
#include <QStringList>
#include <QSortFilterProxyModel>

class CustomFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CustomFilterProxyModel(QObject *parent = nullptr);

    // 设置需要显示的文件夹名称
    void setFolderNameFilter(const QString &workAreaName);

protected:
    // 重写 filterAcceptsRow 方法
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_folderNameFilter;
    QString folders;
    QFileInfoList list;
};

#endif // CUSTOMFILTERPROXYMODEL_H
