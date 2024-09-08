#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QFileDialog>
#include <QtUiTools/QUiLoader>
#include "CustomQTreeWidget.h"
#include "ProcessLine.h"
#include "ImportDialog.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString currentPath;
    QString processlinereadfile;
    QString processlinewritefile;
    QString processlineworking;
    QString destinationPath;
private slots:
    void doubleClick(const QString &filePath);
    void selectFile();
    void openFile();
    void closeFile();
    void new_view(QString *workAreaName);
    void buildTree(const QString &path, QTreeWidgetItem *parentItem);
    void refresh_view(const QString &path,QWidget *parentWidget);
    void listCustomQTreeWidgets();
    void onProcessFinished(const QString &output, const QString &error,  const QString &Path);
    void refresh_tree(const QString &filePath);
    void onTextChanged();
    void saveFile();
    void onImportTriggered();
    void processFileImport(const QStringList &fileInfoList);
    void iterateTreeItems(QTreeWidgetItem* parentItem, const QString& filetype, const QString& filepath);
    void openTipperDialog();
    void Tipper(QStringList &selectedtipperFiles);

private:
    QFileSystemModel *model;
    CustomQTreeWidget *treeView;
    QTabWidget *tabWidget;
    QTextEdit *textEdit;
    QPushButton *backButton;
    QStringList selectFilePathandtype;
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
