#include "FileDialog.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QTreeWidget>
#include "MainWindow.h"
FileDialog::FileDialog(QWidget *parent) : QDialog(parent), mainWindow(static_cast<MainWindow*>(parent)) {
    this->setWindowTitle("Select a File or a data Dir for ProcrssLine");
    this->new_view();
}

void FileDialog::buildTree(const QString &path, QTreeWidgetItem *parentItem) {
    QDir dir(path);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    QFileInfoList fileInfoList = dir.entryInfoList();
    QStringList folderNames = {"resistics","Folder1", "Folder2", "Folder3"};
    QStringList allowedExtensions = {"txt","pmt"};
    for (const QFileInfo &fileInfo : fileInfoList) {
        QString fileExtension = fileInfo.suffix();
        QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
        // qDebug() << fileInfo.absoluteFilePath();
        if (allowedExtensions.contains(fileExtension, Qt::CaseInsensitive) || folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive)){

            item->setText(0, fileInfo.fileName());
            item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
            // qDebug() << fileInfo.absoluteFilePath();
        }

        if (fileInfo.isDir()) {
            if (folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive)){
                buildTree(fileInfo.absoluteFilePath(), item);
            }

        }
    }
}

void FileDialog::new_view(){
    qDebug() << "Selected File:" << mainWindow->currentPath;
    QDir dir_full(mainWindow->currentPath);
    qDebug() << "Selected File:" << dir_full;
    QString directoryName = dir_full.dirName();
    QString directory = dir_full.path();
    CustomQTreeWidget *treeWidget = new CustomQTreeWidget(this);
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabel(directoryName);
    treeWidget->setObjectName(directory);  // 设置 objectName

    // 设置根目录路径
    QString rootPath = mainWindow->currentPath; // 设置为你的文件夹路径
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(treeWidget);
    rootItem->setText(0, rootPath);
    // 构建文件夹结构树
    buildTree(rootPath, rootItem);
    treeWidget->removeItemsBasedOnCondition();
    treeWidget->expandAll();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(treeWidget);
    connect(treeWidget, &QTreeWidget::itemDoubleClicked, this, &FileDialog::itemDoubleClicked);


}

void FileDialog::itemDoubleClicked(QTreeWidgetItem *item, int column) {
    selectedFilePath= item->data(0, Qt::UserRole).toString();
    qDebug() << "Selected File:" << selectedFilePath;

    // 返回选择的文件路径并关闭对话框
    accept();
}


QString FileDialog::getSelectedFilePath() const {
    return selectedFilePath;  // 返回选定的文件路径
}
