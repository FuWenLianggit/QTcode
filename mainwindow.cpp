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
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QFutureWatcher>
#include <Qwt/qwt_symbol.h>
#include <Qwt/qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include "Plot.h"
// #include "CustomFilterProxyModel.h"
#include "ui_mainwindow.h"
#include "customtabbar.h"
#include "ProcessLine.h"
#include "TipperDialog.h"
#include "FileChangeManager.h"
#include "copyworker.h"
#include "progressdialog.h"
#include "CustomScaleDraw.h"
#include "FileDialog.h"
#include "Inverseprocess.h"
#include "TimeSeries.h"
#include "TimeRangeDialog.h"
#include <iostream>
#include <fstream>
#include <cmath>
// 获取当前程序路径 确保运行环境正确
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
    setWindowTitle("航空电磁数据预处理软件");
    this->setFixedSize(1200, 800);
    move(100,0);
    std::string meipassPath = "";
    std::string tmpDir = getTmpDir(meipassPath);
    std::cout << "Temporary Directory: " << tmpDir << std::endl;

    ui->quzao->deleteLater();
    // ui->inverse->deleteLater();

    ui->show_part->setTabBar(new CustomTabBar(ui->show_part));
    ui->show->addWidget(ui->show_part);
    // 按钮
    connect(ui->new_project, &QAction::triggered, this, &MainWindow::selectFile);
    connect(ui->open_project,&QAction::triggered, this, &MainWindow::openFile);
    connect(ui->import_3, &QAction::triggered, this, &MainWindow::onImportTriggered);
    connect(ui->tipper, &QAction::triggered, this, &MainWindow::openTipperDialog);
    connect(ui->close_project_3,&QAction::triggered,this, &MainWindow::closeFile);
    connect(ui->inverse_get_data,&QAction::triggered,this, &MainWindow::handleinverse);


    // QSize size = ui->show_part->size();  // 获取当前大小
    ui->show_part->setFixedSize(1000,750);   // 锁定大小
}

MainWindow::~MainWindow()
{
    delete ui;
}


//点击 预处理-倾子按钮打开界面
void MainWindow::openTipperDialog() {
    TipperDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedtipperFiles = dialog.getSelectedFiles();
        qDebug() <<"tipper selectedFiles" <<selectedtipperFiles ;
        this->Tipper(selectedtipperFiles);
    }
}

//打开导入文件窗口
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

//完成操作 文件-打开工区
void MainWindow::openFile(){
    QString fileName = QFileDialog::getOpenFileName(this, "选择 .json 文件", "", "Info Files (*.json)");
    if (fileName.isEmpty()) {
        return;
    }
    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN","LINlines","Inverse"};
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
        // QString message = "以下文件夹不存在：\n" + missingFolderNames.join("\n") + "\n是否要创建这些文件夹？";
        // QMessageBox::StandardButton reply;
        // reply = QMessageBox::question(this, "创建文件夹", message, QMessageBox::Yes | QMessageBox::No);
        // if (reply == QMessageBox::Yes) {
        //     foreach (const QString &folderName, missingFolderNames) {
        //         if (!dir_open.mkpath(folderName)) {
        //             QMessageBox::warning(this, "创建失败", "无法创建文件夹: " + folderName);
        //         }
        //     }
        // }
        // if (reply == QMessageBox::No){
        //     QMessageBox::warning(this, "缺少文件", "以下文件夹不存在：\n"+missingFolderNames.join("\n")+"\n部分处理可能无法继续");
        // }
        foreach (const QString &folderName, missingFolderNames) {
                    if (!dir_open.mkpath(folderName)) {
                        QMessageBox::warning(this, "创建失败", "无法创建文件夹: " + folderName);
                    }
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

// 实现保存功能 已弃用
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

// 实现保存功能 已弃用
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

// 读取倾子文件和坐标文件
void MainWindow::processDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    QStringList filter;
    filter << "*.tipper";


    // 获取所有 tipper 文件
    QStringList tipperFiles = dir.entryList(filter, QDir::Files);
    if (tipperFiles.size() == 0) return;
    for (const QString& fileName : tipperFiles) {
        QString filePath = dir.absoluteFilePath(fileName);
        readTipperFile(filePath);
    }

    // 读取 coord_distance.txt 文件
    QString coordFilePath = dir.absoluteFilePath("coord_distance.txt");
    if (QFile::exists(coordFilePath)) {
        readCoordDistanceFile(coordFilePath);
    }
}

QDateTime MainWindow::parseFileNameToDateTime(const QString& fileName) {
    // 使用正则表达式解析文件名中的日期时间信息
    QRegularExpression re("(\\d{4})-(\\d{2})-(\\d{2})-(\\d{2})-(\\d{2})-(\\d{2})");
    QRegularExpressionMatch match = re.match(fileName);
    if (match.hasMatch()) {
        int year = match.captured(1).toInt();
        int month = match.captured(2).toInt();
        int day = match.captured(3).toInt();
        int hour = match.captured(4).toInt();
        int minute = match.captured(5).toInt();
        int second = match.captured(6).toInt();

        return QDateTime(QDate(year, month, day), QTime(hour, minute, second));
    }
    return QDateTime();
}

// 读取倾子文件
void MainWindow::readTipperFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << filePath;
        return;
    }

    TipperData data;
    QTextStream in(&file);
    QString firstLine = in.readLine();  // 读取第一行作为频率
    QStringList freqList = firstLine.split(" ");
    for (const QString& freq : freqList) {
        data.frequencies.append(freq.toDouble());
    }

    QString secondLine = in.readLine();  // 读取第二行作为tipperReal
    QStringList realList = secondLine.split(" ");
    for (const QString& real : realList) {
        data.tipperReal.append(real.toDouble());
    }

    QString thirdLine = in.readLine();  // 读取第三行作为tipperImag
    QStringList imagList = thirdLine.split(" ");
    for (const QString& imag : imagList) {
        data.tipperImag.append(imag.toDouble());
    }

    tipperDataList.append(data);
}

// 读取坐标文件
void MainWindow::readCoordDistanceFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open coord_distance file:" << filePath;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (parts.size() >= 4) {
            coordData.time.append(parts[0].toDouble());
            coordData.xdistance.append(parts[1].toDouble());
            coordData.ydistance.append(parts[2].toDouble());
            coordData.distance.append(parts[3].toDouble());
        }
    }
}

//获取定义在类的私有属性的倾子数据
QVector<TipperData> MainWindow::getTipperData() const {
    return tipperDataList;
}

//获取定义在类的私有属性的坐标数据
CoordData MainWindow::getCoordData() const {
    return coordData;
}

//左侧列表项目的双击操作，同时对应 打开 绘制 等操作
//针对于line文件夹处理并绘制倾子
// 针对于LIN.txt绘制飞行轨迹
// 针对于其他不用于绘制用途文件显示内容
// 针对于TS3文件绘制原始数据
//注释部分使用原始方案绘制，渲染可能存在问题
void MainWindow::doubleClick(const QString &filePath) {
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir() && fileInfo.fileName().startsWith("line")) {
        QString filedir = fileInfo.absoluteFilePath();
        QString tabName = fileInfo.fileName();
        qDebug() << "filedir:" << filedir;
        tipperDataList.clear();
        coordData.xdistance.clear();
        coordData.ydistance.clear();
        coordData.distance.clear();
        coordData.time.clear();
        this->processDirectory(filedir);
        QVector<TipperData> tipperDataList = this->getTipperData();
        if (tipperDataList.isEmpty()) return;
        QList<QList<double>> frequencies;
        QList<QList<double>> tipperReal;
        QList<QList<double>> tipperImag;
        for (const TipperData& data : tipperDataList) {
            frequencies.append( data.frequencies.toList());
            tipperReal.append( data.tipperReal.toList());
            tipperImag.append(data.tipperImag.toList());
        }

        CoordData coordData = this->getCoordData();
        QList<double> coordDatas;
        coordDatas = coordData.distance;
        qDebug() << "Coord Distance:" << coordData.distance;
        try{
        //图像模拟
        QWidget *plotWidget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(plotWidget);

        // 创建两个 QwtPlot 实例，一个用于显示实部，一个用于显示虚部 注释部分采用传统绘制
        // QwtPlot *realPlot = new QwtPlot();
        Plot *realPlot = new Plot();
        realPlot->setTitle("Tipper Real Part");
        realPlot->setAxisTitle(QwtPlot::xBottom, "Distance / m");
        realPlot->setAxisTitle(QwtPlot::yLeft, "Tipper real");
        realPlot->insertLegend(new QwtLegend());

        // QwtPlot *imagPlot = new QwtPlot();
        Plot *imagPlot = new Plot();
        imagPlot->setTitle("Tipper Imaginary Part");
        imagPlot->setAxisTitle(QwtPlot::xBottom, "Distance / m");
        imagPlot->setAxisTitle(QwtPlot::yLeft, "Tipper image");
        imagPlot->insertLegend(new QwtLegend());


        QwtPlotGrid* grid = new QwtPlotGrid();
        grid->attach(realPlot);
        QwtPlotGrid* gridimg = new QwtPlotGrid();
        gridimg->attach(imagPlot);


        // // 初始化放大和拖拽功能
        // QwtPlotMagnifier* magnifier = new QwtPlotMagnifier(realPlot->canvas());
        // magnifier->setMouseButton(Qt::NoButton);  // 通过滚轮放大，而不是鼠标按键
        // magnifier->setWheelFactor(1.1);  // 设置滚轮放大的速度
        // QwtPlotPanner* panner = new QwtPlotPanner(realPlot->canvas());
        // panner->setMouseButton(Qt::LeftButton);  // 使用左键拖拽

        // // 对虚部图进行同样设置
        // QwtPlotMagnifier* imagMagnifier = new QwtPlotMagnifier(imagPlot->canvas());
        // imagMagnifier->setMouseButton(Qt::NoButton);
        // imagMagnifier->setWheelFactor(1.1);
        // QwtPlotPanner* imagPanner = new QwtPlotPanner(imagPlot->canvas());
        // imagPanner->setMouseButton(Qt::LeftButton);

        qDebug() << "Check ";
        // 遍历所有频率数据，分别绘制实部和虚部
        for (int i = 0; i < frequencies[0].size(); ++i) {
            const QList<double>& currentFreqReal = tipperReal[i];
            QwtPlotCurve* curve = new QwtPlotCurve(QString("Frequency %1 Hz").arg(frequencies[0][i])); // 假设每个频率下的第一个频率值为标签
            curve->setPen(QPen(QColor::fromHsv(i * 20, 255, 200), 2));
            QVector<double> xData = coordDatas.toVector();
            QVector<double> yData = currentFreqReal.toVector();
            curve->setSamples(xData, yData);
            curve->attach(realPlot);


            const QList<double>& currentFreqimag = tipperImag[i];
            QwtPlotCurve* iamgcurve = new QwtPlotCurve(QString("Frequency %1 Hz").arg(frequencies[0][i])); // 假设每个频率下的第一个频率值为标签
            iamgcurve->setPen(QPen(QColor::fromHsv(i * 20, 255, 200), 2)); // 设置曲线颜色和宽度
            QVector<double> yDataimag = currentFreqimag.toVector();
            iamgcurve->setSamples(xData, yDataimag);
            iamgcurve->attach(imagPlot);

            // realPlot->setSamplesTipper(curve);
            // imagPlot->setSamplesTipper(iamgcurve);
        }
        qDebug() << "Check ";

        // // 遍历所有频率数据，分别绘制实部和虚部
        // for (int i = 0; i < frequencies[0].size(); ++i) {
        //     // 绘制实部曲线
        //     const QList<double>& currentFreqReal = tipperReal[i];
        //     QwtPlotCurve* curve = new QwtPlotCurve(QString("Frequency %1 Hz").arg(frequencies[0][i])); // 假设每个频率下的第一个频率值为标签
        //     curve->setPen(QPen(QColor::fromHsv(i * 20, 255, 200), 1));

        //     // 转换 xData 和 yData 为 QList<QPointF>
        //     QList<QPointF> points;
        //     QVector<double> xData = coordDatas.toVector();
        //     QVector<double> yData = currentFreqReal.toVector();

        //     for (int j = 0; j < xData.size(); ++j) {
        //         points << QPointF(xData[j], yData[j]);
        //     }

        //     curve->setSamples(points);
        //     realPlot->setSamples(curve,points);
        //     curve->attach(realPlot);

        //     // 绘制虚部曲线
        //     const QList<double>& currentFreqImag = tipperImag[i];
        //     QwtPlotCurve* imagCurve = new QwtPlotCurve(QString("Frequency %1 Hz").arg(frequencies[0][i])); // 假设每个频率下的第一个频率值为标签
        //     imagCurve->setPen(QPen(QColor::fromHsv(i * 20, 255, 150), 1)); // 设置曲线颜色和宽度

        //     // 转换 xData 和 yDataimag 为 QList<QPointF>
        //     QList<QPointF> pointsImag;
        //     QVector<double> yDataImag = currentFreqImag.toVector();

        //     for (int j = 0; j < xData.size(); ++j) {
        //         pointsImag << QPointF(xData[j], yDataImag[j]);
        //     }

        //     imagCurve->setSamples(pointsImag);
        //     imagPlot->setSamples(imagCurve,pointsImag);
        //     imagCurve->attach(imagPlot);
        // }

        // 将两个图形添加到布局中
        layout->addWidget(realPlot); // 添加实部图
        layout->addWidget(imagPlot); // 添加虚部图



        if (ui->show_part->count() == 0) {
            ui->show_part->insertTab(0,plotWidget,tabName);
            ui->show_part->removeTab(0);
            ui->show_part->insertTab(0,plotWidget,tabName);
        }else{
            ui->show_part->insertTab(0,plotWidget,tabName);
            ui->show_part->removeTab(-1);
            ui->show_part->insertTab(0,plotWidget,tabName);
        }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << '\n';

        }

    } else if (fileInfo.isFile()) {
        if (fileInfo.fileName().endsWith("LIN.txt")){
            // QProgressDialog progressDialog("绘制中...", "取消", 0, 0, this);
            QProgressDialog *progressDialog =new QProgressDialog("绘制中...", "取消", 0, 0, this);
            progressDialog->setWindowModality(Qt::WindowModal);
            progressDialog->setAutoClose(true);  // 完成后自动关闭
            progressDialog->setAutoReset(true);
            progressDialog->setCancelButton(nullptr);  // 不显示取消按钮
            progressDialog->setWindowTitle("飞行轨迹绘制中...请稍等");
            progressDialog->show();
            QString tabName = fileInfo.fileName();
            QFile file(filePath);

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "Could not open file!";
                return;
            }

            QTextStream in(&file);
            QStringList lines;

            // // Read the entire file and store each line
            while (!in.atEnd()) {
                QString line = in.readLine();
                lines.append(line);
            }

            if (lines.isEmpty()) {
                qDebug() << "File is empty!";
                return;
            }

            // // Step 1: Process the first row to find `0x1010`
            QString firstRow = lines[0];
            QStringList firstRowParts = firstRow.split(QRegularExpression("[\\t\\s]+"), Qt::SkipEmptyParts);

            int timestartpp = -1;
            for (int i = 0; i < firstRowParts.size(); ++i) {
                qDebug() << "firstRowParts[i] :" << firstRowParts[i];
                if (firstRowParts[i] == "0x1010") {
                    timestartpp = i;
                    break;
                }
            }

            if (timestartpp == -1) {
                qDebug() << "0x1010 not found in the first row.";
                return;
            }
            yearandday=firstRowParts[timestartpp+1];
            // //  Extract `timelin` after `0x1010`
            QStringList timelin;
            for (int i = 1; i < lines.size(); ++i) {
                QStringList rowParts = lines[i].split(QRegularExpression("[\\t\\s]+"), Qt::SkipEmptyParts);
                if (rowParts.size() > timestartpp + 2) {
                    timelin.append(rowParts[timestartpp + 2]);
                }
            }

            timelin.removeDuplicates();
            qDebug() << "timelin:" << timelin.size();


            QString projected_coordinates = fileInfo.absolutePath();
            QString fileNameWithoutExtension = fileInfo.baseName();

            qDebug() << "File name without extension:" << fileNameWithoutExtension;
            QString coordinatesfilefilePath = projected_coordinates + "/"+fileNameWithoutExtension+"proj_coords.txt";

            QDir projected_coordinatesdir(projected_coordinates);  // 创建 QDir 对象以表示目录

            // 获取目录中的所有文件和子目录
            QFileInfoList fileInfoList = projected_coordinatesdir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
            QStringList xposition, yposition;

            // 遍历文件列表
            bool ifhere=false;
            for (const QFileInfo& fileInfo : fileInfoList) {

                // qDebug() << "File name in:" << coordinatesfilefilePath << fileInfo.fileName() << "compare to "<<fileNameWithoutExtension+"proj_coords.txt";
                if (fileInfo.fileName() == fileNameWithoutExtension+"proj_coords.txt") {  // 检查文件名是否匹配
                    ifhere = true;
                    qDebug() << "open the File name with extension:" << coordinatesfilefilePath;
                    QFile projected_coordinatesfile(coordinatesfilefilePath);
                    if (!projected_coordinatesfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        qDebug() << "Could not open file!";
                        return;
                    }

                    QTextStream incoordinatesfile(&projected_coordinatesfile);

                    while (!incoordinatesfile.atEnd()) {
                        QString line = incoordinatesfile.readLine();

                        // 按逗号分割行内容
                        QStringList parts = line.split(",");
                        if (parts.size() >= 2) { // 确保有至少两列数据
                            xposition.append(parts[0]); // 保存第一列到xposition
                            yposition.append(parts[1]); // 保存第二列到yposition

                        }
                    }

                    // 关闭文件
                    projected_coordinatesfile.close();
                }

            }

            if(!ifhere){
                qDebug() << "projected_coordinates:" << projected_coordinates;
                QString exePath = QDir::currentPath() + "/bins/inverse.exe";
                QString program = exePath;
                // QString program = "D:/QT6_code/test/bins/inverse.exe";
                QStringList arguments;
                arguments << filePath;
                QDir dir(projected_coordinates);
                // dir.cdUp();
                QString work = dir.path();
                qDebug() << "setWorkingDirectory projected_coordinates"<<  work;
                QProcess *process = new QProcess();
                process->setWorkingDirectory(work);
                process->start(program, arguments);

                if (!process->waitForStarted()) {
                    QString output = process->readAllStandardOutput().data();
                    QString errorOutput = process->readAllStandardError().data();
                    qDebug()<< output << errorOutput;
                    return;
                }

                if (!process->waitForFinished(-1)) {
                    QString output = process->readAllStandardOutput().data();
                    QString errorOutput = process->readAllStandardError().data();
                    qDebug()<< output << errorOutput;
                    return;
                }

                QString output = process->readAllStandardOutput().data();
                QString errorOutput = process->readAllStandardError().data();
                qDebug()<< output << errorOutput;

                QFile projected_coordinatesfile(coordinatesfilefilePath);
                if (!projected_coordinatesfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qDebug() << "Could not open file!";
                    return;
                }

                QTextStream incoordinatesfile(&projected_coordinatesfile);

                while (!incoordinatesfile.atEnd()) {
                    QString line = incoordinatesfile.readLine();

                    // 按逗号分割行内容
                    QStringList parts = line.split(",");
                    if (parts.size() >= 2) { // 确保有至少两列数据
                        xposition.append(parts[0]); // 保存第一列到xposition
                        yposition.append(parts[1]); // 保存第二列到yposition

                    }
                }

                // 关闭文件
                projected_coordinatesfile.close();

            }


            // 将xy坐标更新为从0开始 用来绘制图形
            QStringList xdraw,ydraw;
            if (!xposition.isEmpty()) {
                auto minElement = std::min_element(xposition.begin(), xposition.end());
                int minValue = minElement->toFloat();
                // 打印最小值
                if (minElement != xposition.end()) {
                    std::cout << "Minimum value: " << minElement->toStdString() << std::endl;
                }
                for (auto& str : xposition) {
                    int newValue = str.toFloat() - minValue;
                    xdraw << QString::number(newValue);  // 更新 QStringList 中的值
                }
            }

            if (!yposition.isEmpty()) {
                auto minElement = std::min_element(yposition.begin(), yposition.end());
                int minValue = minElement->toFloat();
                // 打印最小值
                if (minElement != yposition.end()) {
                    std::cout << "Minimum value: " << minElement->toStdString() << std::endl;
                }
                for (auto& str : yposition) {
                    int newValue = str.toFloat() - minValue;
                    ydraw << QString::number(newValue);  // 更新 QStringList 中的值
                }
            }
            qDebug() << "xposition:" << xposition.size();
            qDebug() << "yposition:" << yposition.size();


            QWidget *plotWidget = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(plotWidget);
            Plot *qwtPlot = new Plot();
            // 设置 X 轴名称
            qwtPlot->setAxisTitle(QwtPlot::xBottom, "X / m");

            // 设置 Y 轴名称
            qwtPlot->setAxisTitle(QwtPlot::yLeft, "Y / m");
            // CustomScaleDraw *scaleDraw = new CustomScaleDraw();
            // qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
            // 创建 QwtPlotCurve (示例曲线)
            QwtPlotGrid* grid = new QwtPlotGrid();
            grid->attach(qwtPlot);
            QwtPlotCurve  *curve = new QwtPlotCurve ();
            QVector<QPointF> points;
            // curve->setStyle(QwtPlotCurve::Dots); // 设置样式为散点图
            // curve->setPen( Qt::blue, 3 ),
            //     curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse,
                                              QBrush( Qt::blue ), QPen( Qt::blue, 1 ), QSize( 3, 3 ) );
            curve->setSymbol( symbol );
            curve->setStyle(QwtPlotCurve::NoCurve);  // 只绘制点，不绘制曲线

            for (int i = 0; i < xposition.size(); i++) {
                // qDebug()<<  ts3hz[i][4];
                points << QPointF(xdraw[i].toFloat(), ydraw[i].toFloat());        // 确保 QPointF 的两个参数都是 double
            }

            // points << QPointF(0.0, 0.0) << QPointF(1.0, 2.0) << QPointF(2.0, 1.0) << QPointF(3.0, 3.0);

            curve->setSamples(points);
            qwtPlot->setSamplesLIN(curve,points,timelin);
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
            connect(qwtPlot, &Plot::LINtime, this, &MainWindow::checkLINtime);
            progressDialog->close();
            delete progressDialog;
            progressDialog = nullptr; // 避免悬空指针


        }
        if (fileInfo.fileName().endsWith("tipper") or fileInfo.fileName().endsWith("pmt.txt") or fileInfo.fileName().endsWith("ance.txt")){
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
        if(fileInfo.fileName().endsWith("TS3.txt") ){
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "错误", "无法打开文件");
                return;
            }
            qDebug()<< "douclick clicked file"<<filePath;


            QTextStream in(&file);
            QStringList lines;

            // 读取并修剪空白字符，去除空行
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty()) {
                    lines.append(line);
                }
            }
            file.close(); // 关闭文件
            // 解析开始时间
            QString startTimeString = lines[0].mid(5).trimmed();
            QDateTime startTime = QDateTime::fromString(startTimeString, "yyyy/MM/dd HH:mm:ss");

            // 解析采样频率
            QString freqString = lines[3].split(":")[1].trimmed();
            int sampleFreq = freqString.toInt();

            // QString endTimeString = lines[-(sampleFreq+5)].mid(5).trimmed();
            QDateTime endTime;

            int endIndex = lines.size() - (sampleFreq + 5);
            if (endIndex >= 0 && endIndex < lines.size()) {
                QString endTimeString = lines[endIndex].mid(5).trimmed();
                endTime = QDateTime::fromString(endTimeString, "yyyy/MM/dd HH:mm:ss");
            }
            QVector<QDateTime> ts3time;
            QVector<QVector<double>> ts3hz;
            qDebug() << "startTime"<<  startTime<< "endTime"<< endTime ;
            // 弹出时间选择窗口
            TimeRangeDialog dialog(startTime, endTime);
            if (dialog.exec() == QDialog::Accepted) {
                QProgressDialog *progressDialog =new QProgressDialog("绘制中...", "取消", 0, 0, this);
                // QProgressDialog progressDialog("绘制中...", "取消", 0, 0, this);
                progressDialog->setWindowModality(Qt::WindowModal);
                progressDialog->setAutoClose(true);  // 完成后自动关闭
                progressDialog->setAutoReset(true);
                progressDialog->setCancelButton(nullptr);  // 不显示取消按钮
                progressDialog->setWindowTitle("绘制中...请稍等");
                progressDialog->show();

                // 立即刷新UI以显示弹窗
                QCoreApplication::processEvents();
                // 获取用户选择的时间范围
                QPair<QTime, QTime> selectedTimes = dialog.getSelectedTimeRange();

                QDateTime actualStart = startTime;
                QDateTime actualEnd = endTime;
                actualStart.setTime(selectedTimes.first);
                actualEnd.setTime(selectedTimes.second);
                qDebug()<< "actualStart"<<actualStart<< "actualEnd" << actualEnd;
                // 根据用户选择的时间范围读取数据
                TimeSeries ts3(filePath);

                ts3.readfilepart(actualStart,actualEnd);
                ts3time=ts3.getTimeData();

                ts3hz=ts3.getData();
                qDebug()<< "ts3time.size()"<<ts3time.size()<<ts3hz.size();

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
                    // qDebug()<<  ts3hz[i][4];
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
                progressDialog->close();
                progressDialog->close();
                delete progressDialog;
                progressDialog = nullptr; // 避免悬空指针

                ui->show_part->setCurrentIndex(0);
            }else{
                return;
            }

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




        }

    }
}

// 在飞行轨迹图中选择出两个点后 弹出提示框为测线命名
void MainWindow::checkLINtime(QString starttime,QString endtime,int text1iforxposition,int text2iforxposition){
    qDebug()<< "starttime"<< starttime << "endtime"<<endtime;
    Starttime=starttime;
    Endtime=endtime;

    // 第一个提示窗口，询问用户是否要分割测线
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "分割测线", "是否分割测线？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {

        bool ok;
        QString lineName = QInputDialog::getText(this, "输入测线名称",
                                                 "测线名称 line-:", QLineEdit::Normal, "", &ok);
        if (ok && !lineName.isEmpty()) {

            // QMessageBox::information(this, "测线名称", "您输入的测线名称是: " + lineName);

            linname = lineName;
            //添加测线名称到
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
                this->iterateTreeItemsforLIN(item, linname);  // 调用递归函数

                break;
            }
            foundMatchingType = true;
            if (!foundMatchingType) {
                qDebug() << "No matching filetype found in QTreeWidget.";
            }

        } else {

            QMessageBox::warning(this, "操作取消", "用户取消操作。");
        }
    } else {

        QMessageBox::information(this, "操作", "用户取消操作。");
    }


}

// 循环左侧树中所有的项目 找到LINlines项目，保存相关信息
// LINlines 作为被隐藏的项目，用于存储与飞行轨迹绘制相关的信息，此信息将参与 预处理-倾子 窗口的初始化
void MainWindow::iterateTreeItemsforLIN(QTreeWidgetItem* parentItem, const QString& itemname) {
    if (!parentItem) {

        return;  // parentItem 是空，直接返回
    }

    // 检查当前项是否匹配指定的 filetype
    if (parentItem->text(0)== "LINlines") {
        // 找到匹配的 filetype，创建新的 QTreeWidgetItem
        QTreeWidgetItem* newItem = new QTreeWidgetItem();

        newItem->setText(0, itemname);
        newItem->setData(0, Qt::UserRole, Starttime+","+Endtime);

        // 添加新项到匹配的 QTreeWidgetItem 下
        parentItem->addChild(newItem);
        qDebug()<<"set "<< itemname << "at "<< Starttime+","+Endtime;
        destinationPath = parentItem->data(0, Qt::UserRole).toString();
        return; // 由于找到匹配的 filetype，我们可以退出（除非你想继续查找）
    }

    // 递归检查当前项的所有子项
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem* childItem = parentItem->child(i);
        iterateTreeItemsforLIN(childItem, itemname);  // 递归调用
    }
}

// 完成导入文件后的界面树刷新
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

// 导入文件时进行拷贝行为 包含拷贝和界面更新 进度条更新
void MainWindow::processFileImport(const QStringList &fileInfoList) {
    // 确保列表至少包含两个元素
    if (fileInfoList.size() < 2) {
        qDebug() << "Invalid QStringList size.";
        return;
    }

    QString filetype = fileInfoList[0];
    QString filepath = fileInfoList[1];
    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN","line","LINlines","Inverse"};
    QStringList folderNameszh = {"倾子","空中TS3", "地面TS3", "空中TBL","地面TBL","航线","测线","LINlines","反演结果"};
    int filetypeposition=0;
    for (int a=0;a< folderNames.size();a++){
        if( filetype == folderNames[a]){
            filetypeposition = a;
        }

    }
    filetype = folderNameszh[filetypeposition];
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


    // 源文件路径和目标文件夹路径
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


// 创建左侧工区树结构同时可用于刷新树结构 选择性隐藏了LINlines项目
void MainWindow::buildTree(const QString &path, QTreeWidgetItem *parentItem) {
    QDir dir(path);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    QFileInfoList fileInfoList = dir.entryInfoList();
    // qDebug() << fileInfoList;
    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN","line","LINlines","Inverse"};
    QStringList folderNameszh = {"倾子","空中TS3", "地面TS3", "空中TBL","地面TBL","航线","测线","LINlines","反演结果"};
    QStringList allowedExtensions = {"txt","tipper","dat","rho","prm"};

    for (const QFileInfo &fileInfo : fileInfoList) {
        QString fileExtension = fileInfo.suffix();
        QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
        // qDebug() << fileInfo.absoluteFilePath();
        if (allowedExtensions.contains(fileExtension, Qt::CaseInsensitive) || folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive)
            || fileInfo.fileName().startsWith("line")){
            if (fileInfo.fileName().endsWith("proj_coords.txt"))
            {
                continue;
            }
            if (folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive)){
                int index = folderNames.indexOf(fileInfo.fileName(), 0);  // 获取 fileName 的索引位置
                if (index != -1) {
                    // 处理找到的位置
                    item->setText(0, folderNameszh[index]);
                    item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
                    item->setData(1, Qt::UserRole, fileInfo.fileName());
                    if(fileInfo.fileName().endsWith("TS3")){
                        item->setIcon(0, QIcon(":/images/TS3.png"));
                    }

                    if(fileInfo.fileName().endsWith("LIN")){
                        item->setIcon(0, QIcon(":/images/cutline.png"));
                    }
                    if (fileInfo.fileName().endsWith("Nlines")) {
                        item->setHidden(true);
                    }

                }
            }else
            {
                item->setText(0, fileInfo.fileName());
                item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
                if(fileInfo.fileName().endsWith("TS3")){
                    item->setIcon(0, QIcon(":/images/TS3.png"));
                }

                if(fileInfo.fileName().endsWith("LIN")){
                    item->setIcon(0, QIcon(":/images/cutline.png"));
                }
                if (fileInfo.fileName().endsWith("Nlines")) {
                    item->setHidden(true);
                }
            }
            // if (fileInfo.fileName().startsWith("projected")) {
            //     item->setHidden(true);
            // }
            // qDebug() << fileInfo.absoluteFilePath();
        }

        if (fileInfo.isDir()) {
            if (folderNames.contains(fileInfo.fileName(), Qt::CaseInsensitive) || fileInfo.fileName().startsWith("line")){
                buildTree(fileInfo.absoluteFilePath(), item);

            }

        }
    }
}

// 刷新左侧树
void MainWindow::refresh_tree(const QString &filePath){
    QDir tm(filePath);
    tm.cdUp();
    this->refresh_view(filePath,this);
}

// 创建左侧树的入口 处理树的信号
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


// 测试输出CustomQTreeWidget左侧树的所有项目
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

// 关闭工区，后续需要根据需求增加内存中数据的销毁
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

// 测试输出CustomQTreeWidget左侧树的所有项目
void MainWindow::listCustomQTreeWidgets() {
    printCustomQTreeWidgetNames(this);
}

// 刷新左侧树的视图
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
                // qDebug() << "CustomQTreeWidget objectName:" << treeWidget->objectName();
            }

        }


    }
}

// 创建新的工区
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

        if (!dir.mkpath(fullWorkAreaPath))
        {
            QMessageBox::warning(this, "错误", "无法创建工区文件夹！");
            return;
        }
    } else {
        return;
    }
        // QMessageBox::StandardButton reply;
        // reply = QMessageBox::question(this, "新建工区文件夹",
        //                               "工区文件夹不存在，是否新建?",
        //                               QMessageBox::Yes | QMessageBox::No);
        // if (reply == QMessageBox::Yes) {
        //     if (!dir.mkpath(fullWorkAreaPath)) {
        //         QMessageBox::warning(this, "错误", "无法创建工区文件夹！");
        //         return;
        //     }


    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN","LINlines","Inverse"};
    // 创建工区内的子文件夹
    for (const QString &folderName : folderNames) {
        QDir subDir(fullWorkAreaPath);
        QString folderPath = subDir.filePath(folderName);

        // 检查文件夹是否存在
        if (!subDir.exists(folderPath)) {
            // QMessageBox::StandardButton reply;
            // reply = QMessageBox::question(
            //     nullptr,
            //     "创建文件夹",
            //     QString("文件夹 '%1' 不存在。是否创建它?").arg(folderName),
            //     QMessageBox::Yes | QMessageBox::No,
            //     QMessageBox::No
            //     );

            // if (reply == QMessageBox::Yes) {

                if (subDir.mkdir(folderName)) {
                    std::cout<<'success'<<std::endl;
                    // QMessageBox::information(nullptr, "成功", QString("文件夹 '%1' 已成功创建。").arg(folderName));
                } else {
                    std::cout<<'false'<<std::endl;
                    QMessageBox::critical(nullptr, "错误", QString("无法创建文件夹 '%1'。").arg(folderName));
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

// 预处理-倾子 的程序入口 注释的代码使用另一种方案实现
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
    QDir con(processlineworking);
    processlineworking= con.absolutePath() ;
    QString airTbl = selectedtipperFiles[0];
    QString airTS3 = selectedtipperFiles[1];
    QString groundTbl = selectedtipperFiles[2];
    QString groundTS3 = selectedtipperFiles[3];
    QString LINfile = selectedtipperFiles[4];
    QString LINname = selectedtipperFiles[5];
    QString Splittingtime = selectedtipperFiles[6];
    QDir tmp(processlineworking);
    processlinewritefile=processlineworking+"/"+tmp.dirName()+"pmt.pmt";
    QFile file(processlinewritefile);

    std::replace(Starttime.begin(), Starttime.end(), ':', '-');
    std::replace(Endtime.begin(), Endtime.end(), ':', '-');
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
        out << "line"+linname+"\n";
        out << yearandday+"-"+Starttime+"\n"; //work1
        out << yearandday+"-"+Endtime+"\n";
        // out << "2019-11-19-06-15-50\n";
        // out << "2019-11-19-06-33-50\n";
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
    progressLineDialog =new QProgressDialog("计算中...", "取消", 0, 0, this);
    progressLineDialog->setWindowModality(Qt::WindowModal);
    progressLineDialog->setAutoClose(true);  // 完成后自动关闭
    progressLineDialog->setAutoReset(true);
    progressLineDialog->setCancelButton(nullptr);  // 不显示取消按钮
    progressLineDialog->setWindowTitle("倾子计算中...请稍等");
    progressLineDialog->show();
    if (!processlinereadfile.isEmpty()){
        // Create and start the process thread
        ProcessLine *processThread = new ProcessLine(this);
        connect(processThread, &ProcessLine::finished, this, &MainWindow::onProcessFinished);
        processThread->start();
    }

}

// 处理倾子计算后的相关操作 包括文件拷贝 文件删除
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
            QString destdirPath = Path + "/Tipper";
            QDir dirdes(destdirPath);

            // 检查文件夹是否存在
            if (!dirdes.exists()) {
                if (dirdes.mkpath(destdirPath)) {
                    qDebug() << "Folder created successfully!";
                } else {
                    qDebug() << "Failed to create folder!";
                }
            }
            QThread* thread = new QThread;
            CopyWorker* worker = new CopyWorker(sourcePath, destdirPath);
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

    this->refresh_view(Path,this);

    // 弹出一个提示框
    progressLineDialog->close();
    delete progressLineDialog;
    progressLineDialog = nullptr; // 避免悬空指针
    QMessageBox msgBox;
    msgBox.setWindowTitle("提示");
    msgBox.setText("倾子计算完毕");
    msgBox.setIcon(QMessageBox::Information);  // 设置图标类型为信息图标
    msgBox.setStandardButtons(QMessageBox::Ok);  // 设置“确定”按钮
    msgBox.setDefaultButton(QMessageBox::Ok);    // 默认聚焦“确定”按钮

    // 显示提示框并等待用户点击“确定”
    msgBox.exec();
}


//找到倾子数据所在的line*文件夹
void MainWindow::iterateTreeItemsfindtipper(QTreeWidgetItem* parentItem, QStringList &tipperfolders) {
    if (!parentItem) {

        return;  // parentItem 是空，直接返回
    }

    // 检查当前项是否匹配指定的 filetype
    if (parentItem->text(0)== "倾子") {
        QString tipperfilepath = parentItem->data(0, Qt::UserRole).toString();

        QDir dir(tipperfilepath);
        QStringList filter;
        filter << "line*";


        // 获取所有 tipper 文件夹
        QStringList tipperFiles = dir.entryList(filter, QDir::Dirs );
        if (tipperFiles.size() == 0) return;
        for (const QString& fileName : tipperFiles) {
            tipperfolders << dir.absoluteFilePath(fileName);
            QString filePath = dir.absoluteFilePath(fileName);
            // readTipperFile(filePath);
        }

        // 读取 coord_distance.txt 文件
        // QString coordFilePath = dir.absoluteFilePath("coord_distance.txt");
        // if (QFile::exists(coordFilePath)) {
        //     readCoordDistanceFile(coordFilePath);
        // }

        return;
    }

    // 递归检查当前项的所有子项
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem* childItem = parentItem->child(i);
        iterateTreeItemsfindtipper(childItem,tipperfolders);  // 递归调用
    }
}

double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2) + std::pow(z2 - z1, 2));
}
// 必较数字数量级
bool areSameMagnitude(double a, double b, double tolerance = 1.0) {
    // 计算 a 和 b 的对数值
    double logA = std::log10(std::fabs(a)); // 对数值
    double logB = std::log10(std::fabs(b));
    qDebug() << logA << logB;
    // 计算两个对数值的差异
    return std::fabs(int(logA) - int(logB)) < tolerance; // 判断是否在容忍范围内
}

void MainWindow::readfile_Inv(QStringList linesDIR){


    QDir linedir(linesDIR[0]);
    QString linename = linedir.dirName() + "inv";
    qDebug()<<"linename"<<linename;
    tipperDataList.clear();
    coordData.xdistance.clear();
    coordData.ydistance.clear();
    coordData.distance.clear();
    coordData.time.clear();

    QList<QList<double>> frequencies;
    QList<QList<double>> tipperReal;
    QList<QList<double>> tipperImag;
    QList<double> distance_two_points;
    for (QString &lineDIR : linesDIR){

        this->processDirectory(lineDIR);
        QVector<TipperData> tipperDataList = this->getTipperData();
        if (tipperDataList.isEmpty()) return;

        for (const TipperData& data : tipperDataList) {
            frequencies.append( data.frequencies.toList());
            tipperReal.append( data.tipperReal.toList());
            tipperImag.append(data.tipperImag.toList());
        }

        CoordData coordData = this->getCoordData();
        QList<double> coordDatasx;
        QList<double> coordDatasy;
        QList<double> coordDatasz;
        coordDatasx = coordData.xdistance;
        coordDatasy = coordData.ydistance;
        coordDatasz = coordData.distance;
        for (int  i=0;i< (coordDatasx.size()-1);i++){
            double x1 = coordDatasx[i];
            double x2 = coordDatasx[i+1];

            double y1 = coordDatasy[i];
            double y2 = coordDatasy[i+1];

            double z1 = coordDatasz[i];
            double z2 = coordDatasz[i+1];
            double distance = calculateDistance(x1, y1, z1, x2, y2, z2);
            distance_two_points.append(distance);
        }


    }


    QDir invdir(currentPath);
    QString folderPath = invdir.filePath("Inverse");

    // 检查文件夹是否存在
    if (!invdir.exists(folderPath)) {

        if (invdir.mkdir("Inverse")) {

            // QMessageBox::information(nullptr, "成功", QString("文件夹 '%1' 已成功创建。").arg(folderName));
            invdir.cd("Inverse");

            QString linenamePath = invdir.filePath(linename);
            if (!invdir.exists(linenamePath)){
                if (invdir.mkdir(linename)){
                    qDebug()<<"success";
                }else{
                    QMessageBox::critical(nullptr, "错误", QString("无法创建文件夹 '%1'。").arg(linename));
                }
            }

        } else {

            QMessageBox::critical(nullptr, "错误", QString("无法创建文件夹 '%1'。").arg("Inverse"));
        }

    }else{
        invdir.cd("Inverse");
        QString linenamePath = invdir.filePath(linename);

        if (!invdir.exists(linenamePath)){
            if (invdir.mkdir(linename)){
                qDebug()<<"success";
            }else{
                QMessageBox::critical(nullptr, "错误", QString("无法创建文件夹 '%1'。").arg(linename));
            }
        }
    }

    int nxblocks =100, nyblocks= 62, nsameblocks = 30, nysameblocks = 10;
    if (nsameblocks > nxblocks*0.5) nsameblocks = int(nxblocks*0.3);
    if (int(nxblocks*0.5)!= nxblocks*0.5) nxblocks = nxblocks-1;
    if (int(nyblocks*0.5)!= nyblocks*0.5) nyblocks = nyblocks-1;
    // 数组初始化为均匀剖分部分
    QList<double> xblocks(nsameblocks, int(distance_two_points[0]*(0.5)));
    QList<double> yblocks(nysameblocks, int(200));
    // for (double num : xblocks) {
    //     qDebug() << num ;
    // }
    bool breakbool =false;
    for (int i=0;i < int(nxblocks*0.5) - nsameblocks;i++){
        if (breakbool) break;
        xblocks.append(int(xblocks[xblocks.size()-1]+distance_two_points[0]/(int(nxblocks*0.5) - nsameblocks-i) + xblocks[xblocks.size()-2]) );

        if (areSameMagnitude(xblocks[xblocks.size()-1], xblocks[xblocks.size()-2])) {
            continue;
        } else {
            for (int j=0; j< int(nxblocks*0.5) - nsameblocks - i ; j++){

                int magnitude =std::log10(std::fabs(xblocks[xblocks.size()-1]));
                qDebug() << xblocks[xblocks.size()-1]<< magnitude ;
                double result = std::pow(10, magnitude);
                xblocks.append(int(xblocks[xblocks.size()-1]+ result));
                if (j == (int(nxblocks*0.5) - nsameblocks - i - 1)) breakbool = true;
            }

        }
    }


    bool ybreakbool =false;
    for (int i=0;i < int(nyblocks*0.5) - nysameblocks;i++){
        if (ybreakbool) break;
        yblocks.append(int(yblocks[yblocks.size()-2]+yblocks[0]/(int(nyblocks*0.5) - nysameblocks-i) + yblocks[yblocks.size()-2]*0.5) );
        qDebug() << yblocks[yblocks.size()-2];
        double temp = yblocks[yblocks.size()-2];
        if (areSameMagnitude(yblocks[yblocks.size()-1], yblocks[yblocks.size()-2])) {
            continue;
        } else {
            for (int j=0; j< int(nyblocks*0.5)- nysameblocks - i ; j++){

                int magnitude =std::log10(std::fabs(temp));
                qDebug() << temp<< magnitude ;

                double result = std::pow(10, magnitude);
                temp += result;
                yblocks.append(temp);
                if (j == (int(nyblocks*0.5) - nysameblocks - i - 1)) ybreakbool = true;
            }

        }
    }


    qDebug() << yblocks.size();
    // for (double num : yblocks) {

    //    qDebug() << num;
    // }
    QList<double> xoldblocks = xblocks;

    std::sort(xblocks.begin(), xblocks.end(),std::greater<double>());
    QList<double> xnewBlocks = xblocks;
    for (double num : xoldblocks) {

        xnewBlocks.append(num);
    }


    QList<double> yoldblocks = yblocks;

    // std::sort(yblocks.begin(), yblocks.end(),std::greater<double>());
    QList<double> ynewBlocks = yblocks;
    // for (double num : yoldblocks) {

    //     ynewBlocks.append(num);
    // }

    if (int(nxblocks*0.5)!= nxblocks*0.5){
        int middleIndex = int(xnewBlocks.size() / 2);
        xnewBlocks.insert(middleIndex, distance_two_points[0]);
    }

    if (int(nyblocks*0.5)!= nyblocks*0.5){
        int middleIndex = int(ynewBlocks.size() / 2);
        ynewBlocks.insert(middleIndex, ynewBlocks[ynewBlocks.size() / 2]);
    }


    nyblocks = int(nyblocks*0.5);
    QFile modelfile(currentPath + "/Inverse/"+linename+"/model.rho");

    if (modelfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&modelfile);
        out << QString::number(nxblocks)+ "\t"+ QString::number(nyblocks) + "\t"+ "LOGE" +"\n";
        for (int i = 0; i < int(nxblocks*0.1) ;i++){
            for (int j = 0; j < 10 ;j++){
                out <<"\t" + QString::number(xnewBlocks[i*10 +j]) +  "\t";
                if (j ==9) out <<"\n";
            }
        }
        if (int(nxblocks*0.1) != nxblocks*0.1){
            for (int a = int(nxblocks*0.1)*10; a< nxblocks;a++){
                out <<"\t" + QString::number(xnewBlocks[a]) +  "\t";
                if (a == nxblocks-1)  out <<"\n";
            }
        }


        for (int i = 0; i < int(nyblocks*0.1) ;i++){
            for (int j = 0; j < 10 ;j++){
                out << "\t" +QString::number(ynewBlocks[i*10 +j]) +  "\t";
                if (j ==9) out <<"\n";
            }
        }
        if (int(nyblocks*0.1) != nyblocks*0.1){
            for (int a = int(nyblocks*0.1)*10; a< nyblocks;a++){
                out <<"\t" + QString::number(ynewBlocks[a]) +  "\t";
                if (a == nyblocks-1)  out <<"\n";
            }
        }

        out << QString::number(1)<<"\n";
        for (int a= 0;a< nyblocks;a++){
            for (int i = 0; i < int(nxblocks*0.05) ;i++){
                for (int j = 0; j < 20 ;j++){
                    out <<"\t" + QString::number(1)+  "\t";
                    if (j ==19) out <<"\n";
                }
            }
            if (int(nxblocks*0.05) != nxblocks*0.05){
                for (int a = int(nxblocks*0.2)*20; a< nxblocks;a++){
                    out <<"\t" + QString::number(1) +  "\t";
                    if (a == nxblocks-1)  out <<"\n";
                }
            }
        }

        out << QString::number(100)+  "\t" + QString::number(1e6,'e',6).replace('e','E') <<"\n";

        for (int a= 0;a< nyblocks;a++){
            if (a < int (nyblocks*0.5)){
                out << QString::number(1)+  "\t" ;
            }else{
                out << QString::number(0)+  "\t" ;
                if (a == (nyblocks-1)){
                    out << "\n" ;
                }
            }

        }
        modelfile.close();
    }



    QFile datfile(currentPath + "/Inverse/"+linename+"/data.dat");
    // qDebug() << "datfile" << currentPath + "/Inverse/"+linename+"/data.dat";
    int xspace =int((lineend.toInt()- linestart.toInt())/frequencies.size());
    qDebug() << lineend << "  "<< linestart<< " "<<  xspace;
    if (datfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&datfile);
        out << "# Synthetic 2D MT data written in Matlab\n";
        out << "# Period(s) Code GG_Lat GG_Lon X(m) Y(m) Z(m) Component Real Imag Error\n";
        out << "> Tzy_Impedance\n";
        out << "> exp(-i\\omega t)\n";
        out << "> [V/m]/[T]\n";
        out << "> 0.00\n";
        out << "> 0.000 0.000\n";
        out << "> "+QString::number(frequencies[0].size()) +" "+ QString::number(frequencies.size())+"\n";
        // 写入内容
        double sum = 0.0;
        for (int i = 0;i < frequencies.size(); i++){
            QString iformattedString = QString("%1").arg(i+1, 3, 10, QChar('0'));


            // 累加从第0个元素到第i-1个元素
            if (i != 0) sum += distance_two_points[i-1];

            for (int j = 0 ;j< frequencies[0].size();j++){

                out << QString::number(frequencies[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ iformattedString+ "\t"+ "0.000"+ "\t"+ "0.000"+ "\t"+"0.000"+ "\t"+ QString::number(i == 0 ? (0) : (sum))+ "\t"+ "0.000" + "\t"+ "Ty" + "\t"+ QString::number(tipperReal[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ QString::number(tipperImag[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ QString::number(tipperReal[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\n";
            }
        }

        // out << "# Synthetic 2D MT data written in Matlab\n";
        // out << "# Period(s) Code GG_Lat GG_Lon X(m) Y(m) Z(m) Component Real Imag Error\n";
        // out << "> Full_Vertical_Components\n";
        // out << "> exp(-i\\omega t)\n";
        // out << "> [V/m]/[T]\n";
        // out << "> 0.00\n";
        // out << "> 0.000 0.000\n";
        // out << "> "+QString::number(frequencies[0].size()) +" "+ QString::number(frequencies.size())+"\n";
        // for (int i = 0;i < frequencies.size(); i++){
        //     QString iformattedString = QString("%1").arg(i+1, 3, 10, QChar('0'));
        //     for (int j = 0 ;j< frequencies[0].size();j++){

        //         out << QString::number(frequencies[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ iformattedString+ "\t"+ "0.000"+ "\t"+ "0.000"+ "\t"+"0.000"+ "\t"+ QString::number(i == 0 ? (0) : (sum))+ "\t"+ "0.000" + "\t"+ "TY" + "\t"+ QString::number(tipperReal[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ QString::number(tipperImag[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ QString::number(tipperReal[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\n";
        //     }
        // }
        // 关闭文件
        datfile.close();
        qDebug() << "data File written successfully at "+ currentPath + "/Inverse/"+linename+"/data.dat";
    } else {
        // 文件打开失败时输出错误信息
        qWarning() << "Could not open file for writing:" << datfile.errorString();
    }

    inverseprocessparameters.clear();
    inverseprocessparameters << "-I" << "NLCG" <<currentPath + "/Inverse/"+linename+"/model.rho"  << currentPath + "/Inverse/"+linename+"/data.dat" << "0.001" << "0.000001";
    inverseprocesslineworking = currentPath + "/Inverse/"+linename;
    progressLineDialog =new QProgressDialog("计算中...", "取消", 0, 0, this);
    progressLineDialog->setWindowModality(Qt::WindowModal);
    progressLineDialog->setAutoClose(true);  // 完成后自动关闭
    progressLineDialog->setAutoReset(true);
    progressLineDialog->setCancelButton(nullptr);  // 不显示取消按钮
    progressLineDialog->setWindowTitle("反演计算中...请稍等");
    progressLineDialog->show();
    if (datfile.exists() && modelfile.exists()){

        // Create and start the process thread
        Inverseprocess *inverseprocess = new Inverseprocess(this);
        connect(inverseprocess, &Inverseprocess::finished, this, &MainWindow::onInverseprocessFinished);
        inverseprocess->start();
    }else{
        progressLineDialog->close();
        delete progressLineDialog;
        progressLineDialog = nullptr; // 避免悬空指针
    }

}



// 处理反演计算后的相关操作 包括文件拷贝 文件删除
void MainWindow::onInverseprocessFinished(const QString &output, const QString &error,const QString &Path)
{
    progressLineDialog->close();
    delete progressLineDialog;
    progressLineDialog = nullptr; // 避免悬空指针
    qDebug() << "Output:" << output;
    qDebug() << "Error Output:" << error;
    qDebug() << "Process finished.";

    this->refresh_view(currentPath,this);

    // 弹出一个提示框

    QMessageBox msgBox;
    msgBox.setWindowTitle("提示");
    msgBox.setText("反演计算完毕");
    msgBox.setIcon(QMessageBox::Information);  // 设置图标类型为信息图标
    msgBox.setStandardButtons(QMessageBox::Ok);  // 设置“确定”按钮
    msgBox.setDefaultButton(QMessageBox::Ok);    // 默认聚焦“确定”按钮

    // 显示提示框并等待用户点击“确定”
    msgBox.exec();
}


void MainWindow::handleinverse(){
    QStringList tipperfolders;
    CustomQTreeWidget* customTreeWidget = this->findChild<CustomQTreeWidget*>();
    if (!customTreeWidget) {
        qDebug() << "CustomQTreeWidget not found.";
        return;
    }

    // 遍历 CustomQTreeWidget 中的所有顶层项
    for (int i = 0; i < customTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = customTreeWidget->topLevelItem(i);
        this->iterateTreeItemsfindtipper(item,tipperfolders);

        break;
    }
    qDebug()<< tipperfolders;

    Inversewindows invdialog(this);

    connect(&invdialog, &Inversewindows::returnlineDIR,this, &MainWindow::readfile_Inv);
    if (invdialog.exec() == QDialog::Accepted) {

        qDebug() <<"Inverse close" ;
        // this->Tipper(selectedtipperFiles);
    }

}

