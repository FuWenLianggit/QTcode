#ifndef CUSTOMQTREEWIDGET_H
#define CUSTOMQTREEWIDGET_H

#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>
#include <QThread>
#include <QFileInfo>
#include <QFile>

class CustomQTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit CustomQTreeWidget(QWidget *parent = nullptr);
    QString currentPath;
    QString filePath_work;
    void openAction_op(const QString &filePath);
    void removeItemsBasedOnCondition();
    void removeItemsRecursive(QTreeWidgetItem *item);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void deleteFile(const QString &filePath);
    void refreshmain();

signals:
    void doubleClicked(const QString &filePath);
    void refresh(const QString &filePath);
};

#endif // CUSTOMQTREEWIDGET_H

