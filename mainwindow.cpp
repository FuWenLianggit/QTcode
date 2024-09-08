#include "mainwindow.h"
#include <QDir>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QtUiTools/QUiLoader>
#include "ui_mainwindow.h"
#include "customtabbar.h"
#include <QTabWidget>
#include <QTabBar>
#include <QWidget>
#include <Qwt/qwt.h>
#include <Qwt/qwt_plot.h>
#include <Qwt/qwt_plot_curve.h>
#include <QTreeWidget>
#include <Qwt/qwt_plot_canvas.h>
#include <iostream>
#include <QProcess>
#include <string>
#include <QtConcurrent/QtConcurrent>
#include <filesystem>
#include "Plot.h"
#include "CustomFilterProxyModel.h"
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QFutureWatcher>
#include <Qwt/qwt_symbol.h>
#include "FileDialog.h"
#include <Qwt/qwt_plot_grid.h>
#include "ProcessLine.h"
#include "TipperDialog.h"
#include "FileChangeManager.h"
#include "copyworker.h"
#include "progressdialog.h"
#include "CustomScaleDraw.h"
#include "TimeSeries.h"
std::string getExecutablePath() {
    try {
        // 获取当前程序路径
        std::filesystem::path exePath = std::filesystem::current_path();
        return exePath.string();
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return "";
    }
}


std::string getTmpDir(const std::string& meipassPath = "") {
    if (!meipassPath.empty()) {
        return meipassPath;
    }

    return getExecutablePath();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), model(new QFileSystemModel(this)), tabWidget(new QTabWidget(this)),ui(new Ui::MainWindow){

    ui->setupUi(this);
    setWindowTitle("预处理");
    this->setFixedSize(1200, 800);
    move(100,0);



    ui->show_part->setTabBar(new CustomTabBar(ui->show_part));
    ui->show->addWidget(ui->show_part);
    // 按钮
    connect(ui->new_project, &QAction::triggered, this, &MainWindow::selectFile);
    connect(ui->open_project,&QAction::triggered, this, &MainWindow::openFile);
    connect(ui->import_3, &QAction::triggered, this, &MainWindow::onImportTriggered);
    connect(ui->tipper, &QAction::triggered, this, &MainWindow::openTipperDialog);
    connect(ui->close_project_3,&QAction::triggered,this, &MainWindow::closeFile);
    std::string meipassPath = "";

    std::string tmpDir = getTmpDir(meipassPath);
    std::cout << "Temporary Directory: " << tmpDir << std::endl;

    // QSize size = ui->show_part->size();  // 获取当前大小
    ui->show_part->setFixedSize(980,750);   // 锁定大小
}

MainWindow::~MainWindow()
{
    delete ui;
}

// // 递归函数，将 sourceDirPath 中的内容复制到 destDirPath
// bool copyDirectoryContents(const QString &sourceDirPath, const QString &destDirPath) {
//     QDir sourceDir(sourceDirPath);
//     if (!sourceDir.exists()) {
//         qWarning() << "Source directory does not exist:" << sourceDirPath;
//         return false;
//     }

//     QDir destDir(destDirPath);
//     if (!destDir.exists()) {
//         if (!destDir.mkpath(destDirPath)) {
//             qWarning() << "Failed to create destination directory:" << destDirPath;
//             return false;
//         }
//     }

//     foreach (QString entry, sourceDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
//         QString sourceEntryPath = sourceDir.absoluteFilePath(entry);
//         QString destEntryPath = destDir.absoluteFilePath(entry);

//         QFileInfo entryInfo(sourceEntryPath);
//         if (entryInfo.isDir()) {
//             // 递归复制子文件夹
//             if (!copyDirectoryContents(sourceEntryPath, destEntryPath)) {
//                 return false;
//             }
//         } else {
//             // 复制文件
//             if (!QFile::copy(sourceEntryPath, destEntryPath)) {
//                 qWarning() << "Failed to copy file:" << sourceEntryPath << "to" << destEntryPath;
//                 return false;
//             }
//         }
//     }

//     return true;
// }

// // 处理单个文件复制的函数
// bool copyFile(const QString &sourceFilePath, const QString &destFilePath) {
//     QFileInfo sourceFileInfo(sourceFilePath);
//     if (!sourceFileInfo.exists()) {
//         qWarning() << "Source file does not exist:" << sourceFilePath;
//         return false;
//     }

//     QDir destDir = QFileInfo(destFilePath).absoluteDir();
//     if (!destDir.exists()) {
//         if (!destDir.mkpath(destDir.absolutePath())) {
//             qWarning() << "Failed to create destination directory:" << destDir.absolutePath();
//             return false;
//         }
//     }

//     if (!QFile::copy(sourceFilePath, destFilePath)) {
//         qWarning() << "Failed to copy file:" << sourceFilePath << "to" << destFilePath;
//         return false;
//     }

//     return true;
// }


void MainWindow::openTipperDialog() {
    TipperDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedtipperFiles = dialog.getSelectedFiles();
        qDebug() <<"tipper selectedFiles" <<selectedtipperFiles ;
        this->Tipper(selectedtipperFiles);
    }
}
void MainWindow::onImportTriggered() {
    ImportDialog importDialog(this);
    if (importDialog.exec() == QDialog::Accepted) {
        selectFilePathandtype = importDialog.getSelectedFilePath();
        if (!selectFilePathandtype.isEmpty()) {
            qDebug() << "Selected file path:" << selectFilePathandtype;
            // 在此处处理导入的文件路径
            this->processFileImport(selectFilePathandtype);
        }
    }
}
void MainWindow::openFile(){
    QString fileName = QFileDialog::getOpenFileName(this, "选择 .json 文件", "", "Info Files (*.json)");
    if (fileName.isEmpty()) {
        return;
    }
    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN"};
    QDir dir_open(QFileInfo(fileName).absolutePath());

    bool missingFolders = false;
    QStringList missingFolderNames;

    foreach (const QString &folderName, folderNames) {
        if (!dir_open.exists(folderName)) {
            missingFolders = true;
            missingFolderNames << folderName;
        }
    }
    if (missingFolders) {
        QString message = "以下文件夹不存在：\n" + missingFolderNames.join("\n") + "\n是否要创建这些文件夹？";
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "创建文件夹", message, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            foreach (const QString &folderName, missingFolderNames) {
                if (!dir_open.mkpath(folderName)) {
                    QMessageBox::warning(this, "创建失败", "无法创建文件夹: " + folderName);
                }
            }
        }
        if (reply == QMessageBox::No){
            QMessageBox::warning(this, "缺少文件", "以下文件夹不存在：\n"+missingFolderNames.join("\n")+"\n部分处理可能无法继续");
        }
        QString parent_dir=dir_open.path();
        this->new_view(&parent_dir);
    }else{
        // dir_open.cdUp();
        QString parent_dir=dir_open.path();
        this->new_view(&parent_dir);
    }
    // 打开指定路径的目录
    QDir dir_full(fileName);
    dir_full.cdUp();
    QFileInfoList fileAndFolderList = dir_full.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    qDebug() << "fileAndFolderList " << fileAndFolderList;
    // 创建 .info 文件路径
    QString infoFilePath = dir_full.absolutePath() + "/files_info.json";

    // 初始化 FileChangeManager
    FileChangeManager fileChangeManager(infoFilePath);

    // 在需要的时候，比如程序启动时，初始化文件信息
    fileChangeManager.initializeFileInfo(fileAndFolderList);

    // 定期或者在某个事件发生时检查文件变化并记录
    fileChangeManager.checkAndRecordChanges(fileAndFolderList);
}

void MainWindow::onTextChanged() {
    // 获取 QTextEdit 对象
    QTextEdit *textEdit = this->findChild<QTextEdit *>();

    if (!textEdit) return;  // 如果没有找到 QTextEdit，直接返回
    qDebug()<< "found textEdit";
    // 检查保存按钮是否已经存在
    QPushButton *saveButton = textEdit->findChild<QPushButton *>("saveButton");
    if (!saveButton) {
        saveButton = new QPushButton("保存", textEdit);
        saveButton->setObjectName("saveButton");


        int buttonWidth = 60;
        int buttonHeight = 30;
        int xPosition = textEdit->width() - buttonWidth - 10;
        int yPosition = textEdit->height() - buttonHeight - 10;

        saveButton->setGeometry(xPosition, yPosition, buttonWidth, buttonHeight);
        saveButton->show();

        connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);
    }
}

void MainWindow::saveFile() {
    QTextEdit *textEdit = this->findChild<QTextEdit *>();
    if (!textEdit) return;


    QString content = textEdit->toPlainText();


    // QString filePath = QFileDialog::getSaveFileName(this, "保存文件", "", "Text Files (*.txt);;All Files (*)");

    // if (filePath.isEmpty()) {
    //     return;
    // }

    QString filePath = textEdit->objectName();

    // 使用 QFile 保存文本内容
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件进行写入: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << content;
    file.close();

    QMessageBox::information(this, "文件已保存", "文件已成功保存。");

    // 隐藏保存按钮
    QPushButton *saveButton = textEdit->findChild<QPushButton *>("saveButton");
    if (saveButton) {
        saveButton->hide();
    }
}


void MainWindow::doubleClick(const QString &filePath) {
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir()) {
        return;

    } else if (fileInfo.isFile()) {
        if (fileInfo.fileName().endsWith("json") or fileInfo.fileName().endsWith("pmt.txt")){
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "错误", "无法打开文件");
                return;
            }
            QTextStream in(&file);
            QString content = in.readAll();
            QTextEdit *newTextEdit = new QTextEdit();
            newTextEdit->setText(content);
            QString currentTabText = ui->show_part->tabText(0);
            QString tabName = fileInfo.fileName();
            if (currentTabText == "显示区域") {

                QTextEdit *newTextEdit = new QTextEdit();
                newTextEdit->setText(content);
                connect(newTextEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
                ui->show_part->removeTab(0);

                ui->show_part->insertTab(0, newTextEdit, tabName);
            } else {

                QTextEdit *newTextEdit = new QTextEdit();
                newTextEdit->setText(content);
                newTextEdit->setObjectName(fileInfo.absolutePath());

                int currentIndex = ui->show_part->currentIndex();
                ui->show_part->insertTab(0, newTextEdit, tabName);
                ui->show_part->setCurrentIndex(0);
            }

        }
        if(fileInfo.fileName().endsWith("TS3.txt")){
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "错误", "无法打开文件");
                return;
            }
            qDebug()<< "douclick clicked file"<<filePath;
            TimeSeries ts3(filePath);
            // QVector<QVector<double>> ts3datas= ts3.getdata();
            QVector<QDateTime> ts3time=ts3.getTimeData();

            QVector<QVector<double>> ts3hz=ts3.getData();
            qDebug()<< "ts3time.size()"<<ts3time.size()<<ts3hz.size();


            // QVector<QDateTime> ts3time;
            // QVector<QVector<double>> ts3hz;
            // QThread* thread = new QThread;
            // TimeSeries* ts3worker = new TimeSeries(filePath);


            // ts3worker->moveToThread(thread);

            // QObject::connect(thread, &QThread::started, ts3worker, &TimeSeries::readfile);
            // QObject::connect(ts3worker, &TimeSeries::returndata, [thread, ts3worker,&ts3time,&ts3hz](QVector<QDateTime> timedata,QVector<QVector<double>> data) {

            //     ts3time=timedata;
            //     ts3hz=data;
            //     thread->quit();
            //     thread->wait();

            //     ts3worker->deleteLater();
            //     thread->deleteLater();

            // });

            // thread->start();


            QString tabName = fileInfo.fileName();

            // QTextStream in(&file);
            // QString content = in.readAll();
            // file.close();
            //图像模拟
            QWidget *plotWidget = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(plotWidget);
            Plot *qwtPlot = new Plot();
            // 设置 X 轴名称
            qwtPlot->setAxisTitle(QwtPlot::xBottom, "Time");

            // 设置 Y 轴名称
            qwtPlot->setAxisTitle(QwtPlot::yLeft, "<html>H<sub>z</sub></html>");
            CustomScaleDraw *scaleDraw = new CustomScaleDraw();
            qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
            // 创建 QwtPlotCurve (示例曲线)
            QwtPlotGrid* grid = new QwtPlotGrid();
            grid->attach(qwtPlot);
            QwtPlotCurve  *curve = new QwtPlotCurve ();
            QVector<QPointF> points;
            curve->setPen( Qt::blue, 1 ),
                curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            // QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse,
            //                                   QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
            // curve->setSymbol( symbol );
            for (int i = 0; i < ts3time.size(); i++) {
                double timeInSeconds = ts3time[i].toSecsSinceEpoch(); // 将 QDateTime 转换为秒数
                points << QPointF(timeInSeconds, ts3hz[i][4]);        // 确保 QPointF 的两个参数都是 double
            }

            // points << QPointF(0.0, 0.0) << QPointF(1.0, 2.0) << QPointF(2.0, 1.0) << QPointF(3.0, 3.0);

            curve->setSamples(points);
            qwtPlot->setSamples(curve,points);
            curve->attach(qwtPlot);
            // 将 QwtPlot 添加到布局中
            layout->addWidget(qwtPlot);


            if (ui->show_part->count() == 0) {
                ui->show_part->insertTab(0,plotWidget,tabName);
                ui->show_part->removeTab(0);
                ui->show_part->insertTab(0,plotWidget,tabName);
            }else{
                ui->show_part->insertTab(0,plotWidget,tabName);
                ui->show_part->removeTab(-1);
                ui->show_part->insertTab(0,plotWidget,tabName);
            }
            ui->show_part->setCurrentIndex(0);
        }

    }
}
void MainWindow::iterateTreeItems(QTreeWidgetItem* parentItem, const QString& filetype, const QString& filepath) {
    if (!parentItem) {

        return;  // 如果 parentItem 是空，直接返回
    }

    // 检查当前项是否匹配指定的 filetype
    if (parentItem->text(0)== filetype) {
        // 找到匹配的 filetype，创建新的 QTreeWidgetItem
        QTreeWidgetItem* newItem = new QTreeWidgetItem();
        QFileInfo fileInfo(filepath);
        newItem->setText(0, fileInfo.fileName()); // 设置文本为文件名
        newItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath()); // 设置文件的绝对路径

        // 添加新项到匹配的 QTreeWidgetItem 下
        parentItem->addChild(newItem);
        qDebug()<<"set "<< fileInfo.fileName() << "at "<< parentItem->data(0, Qt::UserRole).toString();
        destinationPath = parentItem->data(0, Qt::UserRole).toString();
        return; // 由于找到匹配的 filetype，我们可以退出（除非你想继续查找）
    }

    // 递归检查当前项的所有子项
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem* childItem = parentItem->child(i);
        iterateTreeItems(childItem, filetype, filepath);  // 递归调用
    }
}


void MainWindow::processFileImport(const QStringList &fileInfoList) {
    // 确保列表至少包含两个元素
    if (fileInfoList.size() < 2) {
        qDebug() << "Invalid QStringList size.";
        return;
    }

    QString filetype = fileInfoList[0];
    QString filepath = fileInfoList[1];

    // 找到 CustomQTreeWidget 对象
    CustomQTreeWidget* customTreeWidget = this->findChild<CustomQTreeWidget*>();
    if (!customTreeWidget) {
        qDebug() << "CustomQTreeWidget not found.";
        return;
    }

    // 遍历 CustomQTreeWidget 中的所有顶层项
    bool foundMatchingType = false;
    for (int i = 0; i < customTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = customTreeWidget->topLevelItem(i);
        this->iterateTreeItems(item, filetype, filepath);  // 调用递归函数
        // if (item->data(0, Qt::UserRole).toString() == filetype) {
        //     // 找到匹配的 filetype，创建新的 QTreeWidgetItem
        //     QTreeWidgetItem* newItem = new QTreeWidgetItem();
        //     QFileInfo fileInfo(filepath);
        //     newItem->setText(0, fileInfo.fileName()); // 设置文本为文件名
        //     newItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath()); // 设置文件的绝对路径

        //     // 添加新项到匹配的 QTreeWidgetItem 下
        //     item->addChild(newItem);

        //     break;
        // }
        break;
    }
    foundMatchingType = true;
    if (!foundMatchingType) {
        qDebug() << "No matching filetype found in QTreeWidget.";
    }


    // 源文件夹路径和目标文件夹路径
    QString sourcePath = filepath;
    QFileInfo sourceInfo(sourcePath);

    QThread* thread = new QThread;
    CopyWorker* worker = new CopyWorker(sourcePath, destinationPath);
    ProgressDialog* progressDialog = new ProgressDialog(nullptr);
    progressDialog->show();
    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, worker, &CopyWorker::startCopy);
    QObject::connect(worker, &CopyWorker::copyFinished, [thread, worker, progressDialog](bool success) {
        // UI 更新应在主线程中进行
        QMetaObject::invokeMethod(progressDialog, [progressDialog]() {
            progressDialog->cancel();
        }, Qt::QueuedConnection);

        if (success) {
            qDebug() << "Copy finished successfully!";
        } else {
            qDebug() << "Copy failed!";
        }

        thread->quit();
        thread->wait();

        worker->deleteLater();
        thread->deleteLater();
        progressDialog->deleteLater();
    });

    QObject::connect(worker, &CopyWorker::progressUpdated, [progressDialog](int percentage) {
        QMetaObject::invokeMethod(progressDialog, [progressDialog, percentage]() {
            progressDialog->setValue(percentage);
        }, Qt::QueuedConnection);
    });


    thread->start();

    progressDialog->exec(); // Show progress dialog modally

    this->refresh_tree(currentPath);
}



void MainWindow::buildTree(const QString &path, QTreeWidgetItem *parentItem) {
    QDir dir(path);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    QFileInfoList fileInfoList = dir.entryInfoList();
    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN","line"};
    QStringList allowedExtensions = {"txt","tipper"};
    for (const QFileInfo &fileInfo : fileInfoList) {
        QString fileExtension = fileInfo.suffix();
        QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
        // qDebug() << fileInfo.absoluteFilePath();
        if (allowedExtensions.contains(fileExtension, Qt::CaseInsensitive) || folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive)
            || fileInfo.fileName().startsWith("line")){

            item->setText(0, fileInfo.fileName());
            item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
            // qDebug() << fileInfo.absoluteFilePath();
        }

        if (fileInfo.isDir()) {
            if (folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive) || fileInfo.fileName().startsWith("line")){
                buildTree(fileInfo.absoluteFilePath(), item);
            }

        }
    }
}

void MainWindow::refresh_tree(const QString &filePath){
    QDir tm(filePath);
    tm.cdUp();
    this->refresh_view(filePath,this);
}

void MainWindow::new_view(QString *fullWorkAreaPath){

    QList<QWidget*> childWidgets = this->findChildren<QWidget*>();

    for (QWidget *widget : childWidgets) {

        if (auto treeWidget = qobject_cast<CustomQTreeWidget*>(widget)) {
            ui->treeview_lay->removeWidget(treeWidget);
            treeWidget->deleteLater();
            ui->show_part->clear();
            qDebug() << "removeAction:" << treeWidget->objectName();
        }
    }

    QDir dir_full(*fullWorkAreaPath);

    QString directoryName = dir_full.dirName();
    QString directory = dir_full.path();
    CustomQTreeWidget *treeWidget = new CustomQTreeWidget(this);
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabel(directoryName);
    treeWidget->setObjectName(directory);
    currentPath=directory;
    // 设置根目录路径
    QString rootPath = *fullWorkAreaPath;
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(treeWidget);
    rootItem->setText(0, rootPath);
    // 构建文件夹结构树
    buildTree(rootPath, rootItem);
    treeWidget->removeItemsBasedOnCondition();
    treeWidget->expandItem(treeWidget->topLevelItem(0));
    ui->treeview_lay->addWidget(treeWidget);
    connect(treeWidget, &CustomQTreeWidget::doubleClicked, this, &MainWindow::doubleClick);
    connect(treeWidget, &CustomQTreeWidget::refresh, this, &MainWindow::refresh_tree);


    // QAction *subAction1 = new QAction(directoryName, this);
    // subAction1->setObjectName(directory);
    // ui->close_project->addAction(subAction1);
    // connect(subAction1, &QAction::triggered, [this, subAction1]() {
    //     closeFile(subAction1, this);
    // });


    // QAction *subAction2 = new QAction(directoryName, this);
    // subAction2->setObjectName(directory);
    // ui->Tipper_2->addAction(subAction2);
    // connect(subAction2,&QAction::triggered, this, &MainWindow::Tipper);
}



void printCustomQTreeWidgetNames(QWidget *parentWidget) {
    // 获取父部件的所有子部件
    QList<QWidget*> childWidgets = parentWidget->findChildren<QWidget*>();


    for (QWidget *widget : childWidgets) {

        if (auto treeWidget = qobject_cast<CustomQTreeWidget*>(widget)) {
            return;
        }

        // 递归
        printCustomQTreeWidgetNames(widget);
    }
}

void MainWindow::closeFile(){

    QList<QWidget*> childWidgets = this->findChildren<QWidget*>();

    for (QWidget *widget : childWidgets) {

        if (auto treeWidget = qobject_cast<CustomQTreeWidget*>(widget)) {

            // if (treeWidget->objectName()==subAction->objectName()){

                ui->treeview_lay->removeWidget(treeWidget);
                // ui->close_project->removeAction(subAction);
                treeWidget->deleteLater();
                ui->show_part->clear();
                qDebug() << "removeAction:" << treeWidget->objectName();
            // }

        }
    }

    // QList<QAction*> actions = parentWidget->findChildren<QAction*>();
    // for (QAction *action : actions) {

    //     if (auto QActio = qobject_cast<QAction*>(action)) {

    //         if (QActio->objectName()==subAction->objectName()){
    //             ui->Tipper_2->removeAction(QActio);
    //             qDebug() << "removeAction:" << QActio->objectName();
    //         }

    //     }
    // }
}

void MainWindow::listCustomQTreeWidgets() {
    printCustomQTreeWidgetNames(this);
}

void MainWindow::refresh_view(const QString &path,QWidget *parentWidget){
    QString rootPath = path;
    qDebug() << "refresh_view rootPath" << rootPath;
    QDir dir_re(path);
    dir_re.cdUp();
    QString parent_dir=dir_re.path();
    // 获取父部件的所有子部件
    QList<QWidget*> childWidgets = parentWidget->findChildren<QWidget*>();

    // 遍历所有子部件
    for (QWidget *widget : childWidgets) {
        if (auto treeWidget = qobject_cast<CustomQTreeWidget*>(widget)) {

            if (treeWidget->objectName()==rootPath){
                treeWidget->clear();

                QTreeWidgetItem *rootItem = new QTreeWidgetItem(treeWidget);
                rootItem->setText(0, rootPath);
                buildTree(rootPath, rootItem);
                treeWidget->removeItemsBasedOnCondition();
                treeWidget->expandItem(treeWidget->topLevelItem(0));
                qDebug() << "CustomQTreeWidget objectName:" << treeWidget->objectName();
            }

        }


    }
}

void MainWindow::selectFile() {

    bool ok;
    QString workAreaName = QInputDialog::getText(this, tr("设置工区命名"),
                                                 tr("请输入工区名称:"), QLineEdit::Normal,
                                                 "", &ok);
    if (!ok || workAreaName.isEmpty())
        return;
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择工区位置");
    if (folderPath.isEmpty())
        return;

    QString fullWorkAreaPath = folderPath + "/" + workAreaName;

    // 检查文件夹是否存在
    QDir dir(fullWorkAreaPath);
    if (!dir.exists()) {

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "新建工区文件夹",
                                      "工区文件夹不存在，是否新建?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (!dir.mkpath(fullWorkAreaPath)) {
                QMessageBox::warning(this, "错误", "无法创建工区文件夹！");
                return;
            }
        } else {
            return;
        }
    }

    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN"};
    // 创建工区内的子文件夹
    for (const QString &folderName : folderNames) {
        QDir subDir(fullWorkAreaPath);
        QString folderPath = subDir.filePath(folderName);

        // 检查文件夹是否存在
        if (!subDir.exists(folderPath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                nullptr,
                "创建文件夹",
                QString("文件夹 '%1' 不存在。是否创建它?").arg(folderName),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
                );

            if (reply == QMessageBox::Yes) {

                if (subDir.mkdir(folderName)) {
                    std::cout<<'success'<<std::endl;
                    // QMessageBox::information(nullptr, "成功", QString("文件夹 '%1' 已成功创建。").arg(folderName));
                } else {
                    std::cout<<'false'<<std::endl;
                    QMessageBox::critical(nullptr, "错误", QString("无法创建文件夹 '%1'。").arg(folderName));
                }
            }
        }
    }

    // 打开指定路径的目录
    QDir dir_full(fullWorkAreaPath);

    QFileInfoList fileAndFolderList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    qDebug() << "fileAndFolderList " << fileAndFolderList;
    // 创建 .info 文件路径
    QString infoFilePath = dir_full.absolutePath() + "/files_info.json";

    // 初始化 FileChangeManager
    FileChangeManager fileChangeManager(infoFilePath);

    // 在需要的时候，比如程序启动时，初始化文件信息
    fileChangeManager.initializeFileInfo(fileAndFolderList);

    // 定期或者在某个事件发生时检查文件变化并记录
    fileChangeManager.checkAndRecordChanges(fileAndFolderList);

    // QFile infoFile(infoFilePath);
    // if (!infoFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //     std::cerr << "Failed to create info file: " << infoFilePath.toStdString() << std::endl;
    //     return;
    // }
    // // 获取文件信息
    // QTextStream out(&infoFile);

    // // 遍历文件列表，获取文件信息并写入 .info 文件
    // foreach (const QFileInfo &fileInfo, fileAndFolderList) {
    //     QString fileName = fileInfo.fileName();
    //     QString creationTime = fileInfo.birthTime().toString(Qt::ISODate);  // 使用 birthTime()
    //     if (!creationTime.isEmpty()) {  // 如果 birthTime 不可用，使用 lastModified
    //         creationTime = fileInfo.lastModified().toString(Qt::ISODate);
    //     }
    //     QString parentDir = fileInfo.absolutePath();
    //     QString fileType = fileInfo.suffix();

    //     out << "File Name: " << fileName << "\n";
    //     out << "Last Modified Time: " << creationTime << "\n";
    //     out << "Parent Folder: " << parentDir << "\n";
    //     out << "File Type: " << fileType << "\n";
    //     out << "-----------------------\n";
    // }

    // infoFile.close();

    this->new_view(&fullWorkAreaPath);
    this->listCustomQTreeWidgets();

    model->setRootPath(folderPath);

}

void MainWindow::Tipper(QStringList &selectedtipperFiles) {
    // 通过唯一的objectname来获得当前工作的工区标识，并且设置计算需要的相关路径信息
    // QAction *action = qobject_cast<QAction *>(sender());

    // if (action) {
    //     // 获取并输出对象的名称
    //     QString actionName = action->objectName();
    //     currentPath=actionName;
    //     processlineworking = actionName;
    //     qDebug() << "Triggered action name:" << actionName;
    // } else {
    //     qDebug() << "Sender is not a QAction";
    // }
    // qDebug() << "currentPath tipper start:" << currentPath;

    // if (selectedtipperFiles.isEmpty()){
    //     return;
    // }


    // FileDialog dialog(this);

    // 选择文件夹或者文件，如果是文件夹判断是否存在数据文件，并将这些文件路径整理成读入文件
    // QString selectedFilePath;
    // if (dialog.exec() == QDialog::Accepted) {
    //     selectedFilePath = dialog.getSelectedFilePath();
    //     qDebug() << "Dialog accepted, file path:" << selectedFilePath;
    // } else {
    //     qDebug() << "Dialog rejected";
    // }

    // QString folderPath = QFileDialog::getExistingDirectory(this, "选择倾子计算数据文件夹");
    // if (folderPath.isEmpty())
    //     return;

    // QDir processlineD(folderPath+'/');

    // qDebug() << "Dialog processlineD"<< processlineD.absolutePath();
    // if (!processlineD.exists()) {
    //     qDebug() << "Directory does not exist:" << processlineD.absolutePath();

    // }

    // processlineD.setFilter(QDir::Files);

    // QStringList nameFilters;
    // nameFilters << "*_TS3.txt" << "*LIN.txt" << "*.txt";
    // QFileInfoList fileList = processlineD.entryInfoList(nameFilters);
    // QString LINfile;
    // QString airTS3;
    // QString groundTS3;
    // QString airTbl;
    // QString groundTbl;
    // int ts3Count = 0;
    // int tblcount = 0;
    // if (!fileList.isEmpty()){
    //     qDebug() << "Directory processlineD:" <<fileList;
    //     for (const QFileInfo &file : fileList) {
    //         QString fileName = file.fileName();

    //         // 检查文件名是否以 "_TS3" 结尾
    //         if (fileName.endsWith("_TS3.txt", Qt::CaseInsensitive)) {
    //             ts3Count++;

    //             if (ts3Count == 1) {
    //                 airTS3 = file.absoluteFilePath();
    //             } else if (ts3Count == 2) {
    //                 groundTS3 = file.absoluteFilePath();
    //                 continue;  // 找到第二个以 "_TS3" 结尾的文件，退出循环
    //             }
    //         }else{
    //             if (!fileName.endsWith("LIN.txt", Qt::CaseInsensitive) && !fileName.contains("_TS", Qt::CaseInsensitive)){
    //                 tblcount++;
    //                 if (tblcount == 1) {
    //                     airTbl = file.absoluteFilePath();
    //                 } else if (tblcount == 2) {
    //                     groundTbl = file.absoluteFilePath();
    //                     continue;
    //                 }
    //             }
    //         }
    //     }

    //     for (QFileInfo file : fileList){
    //         qDebug() << file.fileName();
    //         if (file.fileName().endsWith("LIN.txt", Qt::CaseInsensitive)){
    //             LINfile = file.absoluteFilePath();
    //             qDebug() <<  file.absolutePath()<<LINfile;
    //         }
    //     }
    // }else{
    //     qWarning() << "there is no file end with *_TS3.txt, *LIN.txt and *.txt";
    //     return;
    // }

    QTreeWidget *customTreeWidget = this->findChild<CustomQTreeWidget*>();
    processlineworking = customTreeWidget->objectName();

    QString airTbl = selectedtipperFiles[0];
    QString airTS3 = selectedtipperFiles[1];
    QString groundTbl = selectedtipperFiles[2];
    QString groundTS3 = selectedtipperFiles[3];
    QString LINfile = selectedtipperFiles[4];
    QString Splittingtime = selectedtipperFiles[5];
    QDir tmp(processlineworking);
    processlinewritefile=processlineworking+"/"+tmp.dirName()+"pmt.pmt";
    QFile file(processlinewritefile);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "resistics\n";
        out << processlineworking+"\n";
        out << airTS3+"\n";
        out << groundTS3+"\n";
        out << airTbl+"\n";
        out << groundTbl+"\n";
        out << LINfile+"\n";
        // 写入内容
        // out << "resistics\n";
        // out << processlineworking+"\n";
        // out << "D:/xiongantestdata/test1data/xionganTest/1560bjab_TS3.txt\n";
        // out << "D:/xiongantestdata/test1data/xionganTest/1561bjaa_TS3.txt\n";
        // out << "D:/xiongantestdata/test1data/xionganTest/1560bjab.txt\n";
        // out << "D:/xiongantestdata/test1data/xionganTest/1561bjaa.txt\n";
        // out << "D:/xiongantestdata/test1data/xionganTest/1560BJAB.LIN.txt\n";
        out << "line01\n";
        out << "2019-11-19-06-15-50\n";
        out << "2019-11-19-06-33-50\n";
        out << Splittingtime + "\n";
        out << processlineworking +"\n";
        out << " \n";
        out << " \n";
        out << " \n";
        // 关闭文件
        file.close();
        qDebug() << "File written successfully.";
    } else {
        // 文件打开失败时输出错误信息
        qWarning() << "Could not open file for writing:" << file.errorString();
    }

    // 读入选择的文件或者文件夹整理出的文件
    processlinereadfile=processlinewritefile;
    if (!processlinereadfile.isEmpty()){
        // Create and start the process thread
        ProcessLine *processThread = new ProcessLine(this);
        connect(processThread, &ProcessLine::finished, this, &MainWindow::onProcessFinished);
        processThread->start();
    }

}


void MainWindow::onProcessFinished(const QString &output, const QString &error,const QString &Path)
{
    qDebug() << "Output:" << output;
    qDebug() << "Error Output:" << error;
    qDebug() << "Process finished.";


    QDir refresh(Path);
    QString linedir;

    QDir dir(Path+"/resistics");

    if (dir.exists()) {
        dir.removeRecursively();
    }
    // refresh.cdUp();
    // 检查 result 文件夹是否存在
    if (refresh.cd("result")) { // 进入 result 文件夹
        // 检查 line 文件夹是否存在
        if (refresh.cd("line")) { // 进入 line 文件夹
            // 获取 line 文件夹的绝对路径
            QString lineFolderPath = refresh.absolutePath();
            qDebug() << "Line folder path:" << lineFolderPath;
            linedir= lineFolderPath;

            // 源文件夹路径和目标文件夹路径
            QString sourcePath = linedir;
            QFileInfo sourceInfo(sourcePath);
            destinationPath = Path + "/Tipper";
            QThread* thread = new QThread;
            CopyWorker* worker = new CopyWorker(sourcePath, destinationPath);
            ProgressDialog* progressDialog = new ProgressDialog(nullptr);
            progressDialog->show();
            worker->moveToThread(thread);

            QObject::connect(thread, &QThread::started, worker, &CopyWorker::startCopy);
            QObject::connect(worker, &CopyWorker::copyFinished, [thread, worker, progressDialog](bool success) {
                // UI 更新应在主线程中进行
                QMetaObject::invokeMethod(progressDialog, [progressDialog]() {
                    progressDialog->cancel();
                }, Qt::QueuedConnection);

                if (success) {
                    qDebug() << "Copy finished successfully!";
                } else {
                    qDebug() << "Copy failed!";
                }

                thread->quit();
                thread->wait();

                worker->deleteLater();
                thread->deleteLater();
                progressDialog->deleteLater();
            });

            QObject::connect(worker, &CopyWorker::progressUpdated, [progressDialog](int percentage) {
                QMetaObject::invokeMethod(progressDialog, [progressDialog, percentage]() {
                    progressDialog->setValue(percentage);
                }, Qt::QueuedConnection);
            });


            thread->start();

            progressDialog->exec(); // Show progress dialog modally
        } else {
            qDebug() << "Line folder does not exist.";
        }
    } else {
        qDebug() << "Result folder does not exist.";
    }
    QDir dire(Path+"/result");

    if (dire.exists()) {
        dire.removeRecursively();
    }
    // this->refresh_view(refresh.absolutePath(),this);
}