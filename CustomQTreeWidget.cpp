#include "CustomQTreeWidget.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <iostream>
#include <QTreeWidget>
CustomQTreeWidget::CustomQTreeWidget(QWidget *parent) : QTreeWidget(parent), currentPath(QDir::rootPath()) {}

void CustomQTreeWidget::contextMenuEvent(QContextMenuEvent *event) {

    QTreeWidgetItem *item = itemAt(event->pos());
    if (!item) {
        return;
    }
    QString filePath = item->data(0, Qt::UserRole).toString();
    filePath_work = this->objectName();
    qDebug() << "filePath_work" << filePath_work;
    QFileInfo fileInfo(filePath);
    // 仅在文件上点击时显示菜单
    if (QDir(filePath).exists() && fileInfo.fileName().startsWith("line")){
        QMenu menu(this);
        QAction *draw = new QAction("绘制", this);
        QAction *deleteAction = new QAction("删除", this);
        connect(deleteAction, &QAction::triggered, [this, filePath]() { deleteFile(filePath); });
        connect(draw, &QAction::triggered, [this, filePath]() { openAction_op(filePath); });
        menu.addAction(draw);
        menu.addAction(deleteAction);
        menu.exec(event->globalPos());
        return;
    }
    if (QDir(filePath).exists()) return;

    QMenu menu(this);

    QAction *openAction = new QAction("打开", this);
    if (fileInfo.fileName().endsWith(".LIN.txt")){
        openAction->setText("测线分割");
    }
    QAction *deleteAction = new QAction("删除", this);
    QAction *propertiesAction = new QAction("属性", this);
    connect(openAction, &QAction::triggered,[this, filePath]() { openAction_op(filePath); });
    connect(deleteAction, &QAction::triggered, [this, filePath]() { deleteFile(filePath); });

    menu.addAction(openAction);
    menu.addAction(deleteAction);
    menu.addAction(propertiesAction);

    menu.exec(event->globalPos());
}

void CustomQTreeWidget::openAction_op(const QString &filePath){
    emit doubleClicked(filePath);
}

void CustomQTreeWidget::removeItemsBasedOnCondition()
{
    // 获取根项目的数量
    int topLevelItemCount = this->topLevelItemCount();
    // qDebug() << topLevelItemCount;
    for (int i = 0; i < topLevelItemCount; ++i) {
        QTreeWidgetItem *topItem = this->topLevelItem(i);

        removeItemsRecursive(topItem);
    }
}


void CustomQTreeWidget::removeItemsRecursive(QTreeWidgetItem *item)
{
    if (!item) return;

    for (int i = item->childCount() - 1; i >= 0; --i) {
        QTreeWidgetItem *child = item->child(i);
        // qDebug() << "data"<<child->data(0, Qt::UserRole).toString();
        removeItemsRecursive(child);


        if (child->data(0, Qt::UserRole).toString()=="") {
            delete item->takeChild(i);
        }
    }


    // if (item->data(0, Qt::UserRole).toString()=="") {
    //     delete item;
    // }
}

void CustomQTreeWidget::mouseDoubleClickEvent(QMouseEvent *event) {

    QTreeWidgetItem *item = itemAt(event->pos());
    if (!item) {
        return;
    }
    QString filePath = item->data(0, Qt::UserRole).toString();
    QFileInfo fileInfo(filePath);

    // if (fileInfo.isDir()) {

    //     // 检查是否已经展开
    //     bool isExpanded = this->isItemExpanded(item);
    //     // 展开或折叠当前项
    //     this->setItemExpanded(item, !isExpanded);

    // } else {
    // qDebug() << "isDir:" << fileInfo.filePath();
    emit doubleClicked(filePath);
    // }

    QTreeWidget::mouseDoubleClickEvent(event);
}

void CustomQTreeWidget::deleteFile(const QString &filePath) {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "删除文件", QString("确定删除 \"%1\" 吗？").arg(filePath),
                                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            QFile::remove(filePath);
        } else if (fileInfo.isDir()) {
            QDir dir(filePath);
            dir.removeRecursively();
        } else {
            QMessageBox::warning(this, "错误", "无法删除该文件。");
        }
    }
    emit refresh(filePath_work);
}
