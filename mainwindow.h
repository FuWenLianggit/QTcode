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

struct TipperData {
    QVector<double> frequencies;
    QVector<double> tipperReal;
    QVector<double> tipperImag;
};

struct CoordData {
    QVector<double> time;
    QVector<double> distance;
};
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QVector<TipperData> getTipperData() const;
    CoordData getCoordData() const;
    void processDirectory(const QString& dirPath);
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
    void iterateTreeItemsforLIN(QTreeWidgetItem* parentItem, const QString& itemname);
    void openTipperDialog();
    void Tipper(QStringList &selectedtipperFiles);
    void checkLINtime(QString starttime,QString endtime);
private:
    QFileSystemModel *model;
    CustomQTreeWidget *treeView;
    QTabWidget *tabWidget;
    QTextEdit *textEdit;
    QPushButton *backButton;
    QStringList selectFilePathandtype;
    Ui::MainWindow *ui;
    QVector<TipperData> tipperDataList;
    CoordData coordData;

    QDateTime parseFileNameToDateTime(const QString& fileName);
    QString Starttime;
    QString Endtime;
    QString linname;
    QString yearandday;
    void readTipperFile(const QString& filePath);
    void readCoordDistanceFile(const QString& filePath);

};

#endif // MAINWINDOW_H
