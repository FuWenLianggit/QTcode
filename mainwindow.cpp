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
#include <QToolBar>
#include <QStatusBar>
#include <QToolButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
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
#include <QPainter>
#include <QMenu>
#include <QFutureWatcher>
#include <Qwt/qwt_symbol.h>
#include <Qwt/qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_magnifier.h>
#include <qwt_scale_engine.h>
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
#include "Plotcontour.h"
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
    QString targetFilePath = currentPath + "/" +"config.ini";


    QFile file(targetFilePath);

    if (!file.exists()) {
        qDebug() << "File does not exist:" << targetFilePath;

    }

    if (file.remove()) {
        qDebug() << "File deleted successfully:" << targetFilePath;

    } else {
        qDebug() << "Failed to delete file:" << targetFilePath;

    }

    QString targetFilePathre = currentPath + "/" +"resisticsConfig.ini";
    QFile filere(targetFilePathre);
    if (!filere.exists()) {
        qDebug() << "File does not exist:" << targetFilePathre;

    }

    if (filere.remove()) {
        qDebug() << "File deleted successfully:" << targetFilePathre;

    } else {
        qDebug() << "Failed to delete file:" << targetFilePathre;

    }
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
    in.readLine();
    QString FifthLine = in.readLine();  // 读取第5行作为tipperReal
    QStringList realListzy = FifthLine.split(" ");
    for (const QString& real : realListzy) {
        data.tipperRealzy.append(real.toDouble());
    }

    QString sixthLine = in.readLine();  // 读取第6行作为tipperImag
    QStringList imagListzy = sixthLine.split(" ");
    for (const QString& imag : imagListzy) {
        data.tipperImagzy.append(imag.toDouble());
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
// 针对于rho文件第一列为x 第二列为y 第三列为计算出的结果 调整文件中原始数据的范围 绘制从0开始的等值线图
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
        QList<QList<double>> tipperRealzy;
        QList<QList<double>> tipperImagzy;
        for (const TipperData& data : tipperDataList) {
            frequencies.append( data.frequencies.toList());
            tipperReal.append( data.tipperReal.toList());
            tipperImag.append(data.tipperImag.toList());
            tipperRealzy.append(data.tipperRealzy.toList());
            tipperImagzy.append(data.tipperImagzy.toList());
        }

        QList<QList<double>> tipperAmplitude(tipperReal); // 用于存储计算出的幅值
        // 确保 tipperReal 和 tipperImag 的大小一致
        if (tipperReal.size() == tipperImag.size()) {


            for (int i = 0; i < tipperReal.size(); ++i) {
                for(int j=0;j< tipperReal[i].size();++j){
                    double real = tipperReal[i][j];
                    double imag = tipperImag[i][j];
                    double amplitude = std::sqrt(real * real + imag * imag); // 计算幅值
                    tipperAmplitude[i][j] = amplitude;
                }

            }
        }

        QList<QList<double>> tipperAmplitudezy(tipperReal); // 用于存储计算出的幅值
        // 确保 tipperReal 和 tipperImag 的大小一致
        if (tipperRealzy.size() == tipperImagzy.size()) {


            for (int i = 0; i < tipperRealzy.size(); ++i) {
                for(int j=0;j< tipperRealzy[i].size();++j){
                    double real = tipperRealzy[i][j];
                    double imag = tipperImagzy[i][j];
                    double amplitude = std::sqrt(real * real + imag * imag); // 计算幅值
                    tipperAmplitudezy[i][j] = amplitude;
                }

            }
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
        Plot *realPlot = new Plot(false);
        realPlot->setTitle("Tipper Amplitude Tzx");
        realPlot->setAxisTitle(QwtPlot::xBottom, "Distance / m");
        realPlot->setAxisTitle(QwtPlot::yLeft, "Amplitude");
        realPlot->insertLegend(new QwtLegend());

        // QwtPlot *imagPlot = new QwtPlot();
        Plot *imagPlot = new Plot(false);
        imagPlot->setTitle("Tipper Amplitude Tzy");
        imagPlot->setAxisTitle(QwtPlot::xBottom, "Distance / m");
        imagPlot->setAxisTitle(QwtPlot::yLeft, "Amplitude");
        imagPlot->insertLegend(new QwtLegend());

        realPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
        imagPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());

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
            const QList<double>& currentFreqReal = tipperAmplitude[i];
            QwtPlotCurve* curve = new QwtPlotCurve(QString("Frequency %1 Hz").arg(frequencies[0][i])); // 假设每个频率下的第一个频率值为标签
            curve->setPen(QPen(QColor::fromHsv(i * 20, 255, 200), 2));
            QVector<double> xData = coordDatas.toVector();
            QVector<double> yData = currentFreqReal.toVector();
            curve->setSamples(xData, yData);
            curve->attach(realPlot);


            const QList<double>& currentFreqimag = tipperAmplitudezy[i];
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
            Plot *qwtPlot = new Plot(false);
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
                // for(int i =1;i<10;i++){
                //     qDebug()<<ts3time[ts3time.size()-i];
                // }
                QString tabName = fileInfo.fileName();

                // QTextStream in(&file);
                // QString content = in.readAll();
                // file.close();
                //图像模拟
                QWidget *plotWidget = new QWidget();
                QVBoxLayout *layout = new QVBoxLayout(plotWidget);
                Plot *qwtPlot = new Plot(false);
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
                for (int i = 0; i < ts3time.size() -sampleFreq; i++) {

                    double timeInSeconds = ts3time[i].toSecsSinceEpoch(); // 将 QDateTime 转换为秒数
                    // qDebug()<<  ts3hz[i][4];
                    points << QPointF(timeInSeconds, ts3hz[i][4]);        // 确保 QPointF 的两个参数都是 double
                }

                // points << QPointF(0.0, 0.0) << QPointF(1.0, 2.0) << QPointF(2.0, 1.0) << QPointF(3.0, 3.0);

                curve->setSamples(points);
                qwtPlot->setSamplesTipper(curve,points);
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

        if(fileInfo.fileName().endsWith(".rho") ){
            QFile datfile(filePath);
            if (!datfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "错误", "无法打开文件");
                return;
            }
            qDebug()<< "douclick clicked file"<<filePath;

            QVector<double> x_coords;
            QVector<double> y_coords;
            QVector<QVector<double>> dataread;



            QString tabName = fileInfo.fileName();

            xblocks.clear();
            yblocks.clear();
            data.clear();
            xblocks.push_back(0);
            yblocks.push_back(0);

            QTextStream in(&datfile);
            std::vector<double> temp_x_coords; // 用于暂存 x 坐标
            std::vector<double> temp_x_coordsfinal;
            std::vector<double> temp_y_coords; // 用于暂存 y 坐标

            std::vector<std::vector<double>>  temp_data;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                QStringList values = line.split("\t");
                if (values.size() != 3) continue;

                // 提取 x, y, 和 data 值
                double x = values[0].toDouble();
                double y = values[1].toDouble();
                double value = values[2].toDouble();

                // 如果 x 和 y 是新的，则添加到临时列表中
                if (temp_x_coords.size() == 0 || temp_x_coords.back() != x) {
                    temp_x_coords.push_back(x);
                }
                if (temp_y_coords.size() == 0 || temp_y_coords.back() != y) {
                    temp_y_coords.push_back(y);
                    temp_data.push_back(std::vector<double>()); // 为新行创建存储空间
                }

                // 将值添加到对应的行
                temp_data.back().push_back(value);
            }

            datfile.close();
            for(int i = 0;i<temp_data[0].size();i++){
                temp_x_coordsfinal.push_back(temp_x_coords[i]);
            }
            double stand = temp_x_coordsfinal[0];
            for(int i = 0;i< temp_x_coordsfinal.size();i++){
                temp_x_coordsfinal[i]-=stand;
            }
            double standy = temp_y_coords[0];
            for(int i = 0;i< temp_y_coords.size();i++){
                temp_y_coords[i]-=standy;
            }
            qDebug() << temp_x_coordsfinal.size()<< temp_y_coords.size() << temp_data.size() << temp_data[0].size();
            // for (int i = 0;i<temp_x_coords.size()-1;i++){
            //     xblocks.append(temp_x_coords[i+1]);
            // }

            // QTextStream in(&file);
            // QString line = in.readLine();
            // QStringList tokens =  line.split(' ', Qt::SkipEmptyParts);
            // qDebug()<<tokens<<filePath;

            // int xblock = tokens[0].toInt();
            // int yblock = tokens[1].toInt();






            // // 读取 xblocks
            // while (!in.atEnd()) {
            //     line = in.readLine();
            //     QStringList numbers = line.split(' ', Qt::SkipEmptyParts);
            //     for (const QString& num : numbers) {
            //         xblocks.push_back(num.toDouble());
            //         if (xblocks.size() == xblock) {
            //             break;
            //         }
            //     }
            //     if (xblocks.size() == xblock) break;
            // }

            // // 读取 yblocks
            // while (!in.atEnd()) {
            //     line = in.readLine();
            //     QStringList numbers = line.split(' ', Qt::SkipEmptyParts);
            //     for (const QString& num : numbers) {
            //         yblocks.push_back(num.toDouble());
            //         if (yblocks.size() == yblock) {
            //             break;
            //         }
            //     }
            //     if (yblocks.size() == yblock) break;
            // }

            // line = in.readLine();
            // // 读取 data
            // std::vector<double> temp;
            // while (!in.atEnd()) {
            //     line = in.readLine();
            //     QStringList numbers = line.split(' ', Qt::SkipEmptyParts);
            //     for (const QString& num : numbers) {
            //         temp.push_back(num.toDouble());
            //         if (temp.size() == xblock) {
            //             data.push_back(temp);
            //             temp.clear();
            //         }
            //     }
            // }
            // for (int a =0;a< xblocks.size() ;a++){
            //     qDebug()<< xblocks[a];
            // }

            // for (int a =0;a< yblocks.size() ;a++){
            //     qDebug()<< yblocks[a];
            // }
            // qDebug()<< data[0];
            // 创建并绘制内容
            QWidget *plotWidget = new QWidget();
            Plotcontour* plot = new Plotcontour(temp_x_coordsfinal,temp_y_coords,temp_data);
            plot->setContentsMargins( 0, 5, 0, 10 );

            // setCentralWidget( plot );

// #ifndef QT_NO_PRINTER
//             QToolButton* btnPrint = new QToolButton();
//             btnPrint->setText( "Print" );
//             btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
//             connect( btnPrint, SIGNAL(clicked()), plot, SLOT(printPlot()) );
// #endif

//             QComboBox* mapBox = new QComboBox();
//             mapBox->addItem( "RGB" );
//             mapBox->addItem( "Hue" );
//             mapBox->addItem( "Saturation" );
//             mapBox->addItem( "Value" );
//             mapBox->addItem( "Sat.+Value" );
//             mapBox->addItem( "Alpha" );
//             mapBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
//             connect( mapBox, SIGNAL(currentIndexChanged(int)),
//                     plot, SLOT(setColorMap(int)) );


//             QComboBox* colorTableBox = new QComboBox();
//             colorTableBox->addItem( "None" );
//             colorTableBox->addItem( "256" );
//             colorTableBox->addItem( "1024" );
//             colorTableBox->addItem( "16384" );
//             connect( colorTableBox, SIGNAL(currentIndexChanged(int)),
//                     plot, SLOT(setColorTableSize(int)) );

//             QSlider* slider = new QSlider( Qt::Horizontal );
//             slider->setRange( 0, 255 );
//             slider->setValue( 255 );
//             connect( slider, SIGNAL(valueChanged(int)), plot, SLOT(setAlpha(int)) );

//             QCheckBox* btnSpectrogram = new QCheckBox( "Spectrogram" );
//             btnSpectrogram->setChecked( true );
//             connect( btnSpectrogram, SIGNAL(toggled(bool)),
//                     plot, SLOT(showSpectrogram(bool)) );

//             QCheckBox* btnContour = new QCheckBox( "Contour" );
//             btnContour->setChecked( false );
//             connect( btnContour, SIGNAL(toggled(bool)),
//                     plot, SLOT(showContour(bool)) );

//             QToolBar* toolBar = new QToolBar();

// // #ifndef QT_NO_PRINTER
// //             toolBar->addWidget( btnPrint );
// //             toolBar->addSeparator();
// // #endif
//             toolBar->addWidget( new QLabel("Color Map " ) );
//             toolBar->addWidget( mapBox );

//             toolBar->addWidget( new QLabel("Table " ) );
//             toolBar->addWidget( colorTableBox );

//             toolBar->addWidget( new QLabel( " Opacity " ) );
//             toolBar->addWidget( slider );

//             toolBar->addWidget( new QLabel("   " ) );
//             toolBar->addWidget( btnSpectrogram );
//             toolBar->addWidget( btnContour );

//             addToolBar( toolBar );

//             connect( plot, SIGNAL(rendered(const QString&)),
//                     statusBar(), SLOT(showMessage(const QString&)),
//                     Qt::QueuedConnection );

            // // 创建 QLabel 控件以显示图像
            QVBoxLayout *layout = new QVBoxLayout(plotWidget);

            // layout->addWidget(toolBar);
            // // 将 QwtPlot 添加到布局中
            layout->addWidget(plot);


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
    }
}




void MainWindow::draw_contour(QPainter *painter) {
    // 计算 x 和 y 坐标
    std::vector<double> x_coords = {0};
    std::partial_sum(xblocks.begin(), xblocks.end(), std::back_inserter(x_coords));

    std::vector<double> y_coords = {0};
    std::partial_sum(yblocks.begin(), yblocks.end(), std::back_inserter(y_coords));

    // 调整 x 坐标值，使其对称
    double x_sum = x_coords.back();
    for (size_t i = 0; i < x_coords.size(); ++i) {
        x_coords[i] -= x_sum / 2;
        qDebug() << x_coords[i];
    }
    double min_resistivity,max_resistivity ;
    // 绘制网格和等值线
    double min_resistivitytemp =10000000,max_resistivitytemp =0 ;
    for (int i = 0; i< data.size();i++){
        min_resistivity = *std::min_element(data[i].begin(), data[i].end());
        max_resistivity = *std::max_element(data[i].begin(), data[i].end());

        if (min_resistivity < min_resistivitytemp) min_resistivitytemp = min_resistivity;
        if (max_resistivity > max_resistivitytemp) max_resistivitytemp = max_resistivity;
    }

    qDebug() << data[0];
    qDebug() << min_resistivitytemp << max_resistivitytemp;
    min_resistivity = min_resistivitytemp;
    max_resistivity = max_resistivitytemp;
    painter->setPen(Qt::black);
    int num_levels = 20; // 等值线的层数
    double level_step = (max_resistivity - min_resistivity) / num_levels;

    // 绘制数据的等值线
    for (size_t i = 0; i < data.size() - 1; ++i) {
        for (size_t j = 0; j < data[i].size() - 1; ++j) {
            double z1 = data[i][j];
            double z2 = data[i + 1][j];
            double z3 = data[i][j + 1];
            double z4 = data[i + 1][j + 1];
            double temp=1000000000000;
            if(z1 < temp) temp = z1;
            if(z2 < temp) temp = z2;
            if(z3 < temp) temp = z3;
            if(z4 < temp) temp = z4;

            // 计算每个块的平均值
            double avg_z = temp;

            // 将平均值映射到颜色渐变范围
            double normalized_value = (avg_z - min_resistivity) / (max_resistivity - min_resistivity);
            // normalized_value = qBound(0.0, normalized_value, 1.0); // 限制在[0,1]范围内


            // 生成从蓝色到黄色的渐变颜色
            int hue = static_cast<int>(240 - normalized_value * (240 - 60)); // 从240(蓝色)到60(黄色)
            QColor color = QColor::fromHsl(hue, 255, 128); // 亮度固定为128，饱和度255
            // qDebug() << "normalized_value:" << normalized_value << ", hue:" << hue;

            // 绘制矩形块
            QRectF rect(x_coords[j], y_coords[i], x_coords[j + 1] - x_coords[j], y_coords[i + 1] - y_coords[i]);

            painter->fillRect(rect, color);
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

// 完成导入文件后的界面树刷新 这里使用的filetype即为倾子 空中TBL此类描述
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
    QStringList folderNameszh = {"倾子","空中TS3", "地面TS3", "空中TBL","地面TBL","飞行轨迹","测线","LINlines","反演结果"};
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
        // UI 更新应在主线程中进行 事实上未在主界面的私有属性中定义的更新机制不会在主进程刷新
        // 此代码功能未完善 但不存在问题
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


// 创建左侧工区树结构同时可用于刷新树结构 选择性隐藏了LINlines项目 LINlines项目存在于文件夹，可以在结束时删除，作为临时存储测线信息的文件夹，用以与倾子计算界面交互
void MainWindow::buildTree(const QString &path, QTreeWidgetItem *parentItem) {
    QDir dir(path);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    QFileInfoList fileInfoList = dir.entryInfoList();
    // qDebug() << fileInfoList;
    QStringList folderNames = {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN","line","LINlines","Inverse"};
    QStringList folderNameszh = {"倾子","空中TS3", "地面TS3", "空中TBL","地面TBL","飞行轨迹","测线","LINlines","反演结果"};
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

    // 将软件内存中的配置文件拷贝到当前的工作目录，config存在乱码可能性，直接尝试写入
    // 这些文件在预处理中作为配置文件，在软件关闭时销毁
    QDir targetDirectory(currentPath);

    QString targetFilePath = currentPath + "/" +"config.ini";
    QString sourceFilePath = QDir::currentPath() + "/bins/config.ini";
    QFile file(targetFilePath);

    if (!file.exists()) {
        ;

    }

    if (file.remove()) {
        ;

    } else {
        ;

    }
    // 复制文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream outFile (&file);
        outFile << "name = config\n\n";
        outFile << "[Decimation]\n";
        outFile << "numlevels = 9\n\n";
        outFile << "[Frequencies]\n";
        outFile << "frequencies = 10000, 7305.27, 5336.7, 3898.6, 2848.04, 2080.57, 1519.91, 1110.34, 811.131, 580, 420, 320, 230, 160, 120, 90, 75, 37, 20, 17, 15, 13.6887, 10, 7.30527, 5.3367, 3.8986, 2.84804, 2.08057, 1.51991, 1.11034, 0.811131, 0.592553, 0.432876, 0.316228, 0.231013, 0.168761, 0.123285, 0.0900628, 0.0657933, 0.0480638, 0.0351119, 0.0256502, 0.0187382, 0.0136887, 0.01\n";
        outFile << "perlevel = 5\n\n";
        outFile << "[Window]\n";
        outFile << "minwindowsize = 256\n";
        outFile << "minoverlapsize = 64\n";

        // 关闭文件
        file.close();}
    // if (QFile::copy(sourceFilePath, targetFilePath)) {
    //     ;
    // } else {
    //     ;
    // }

    QString targetFilePathre = currentPath + "/" +"resisticsConfig.ini";
    QString sourceFilePathre = QDir::currentPath() + "/bins/resisticsConfig.ini";
    QFile filere(targetFilePathre);

    if (!filere.exists()) {
        ;

    }

    if (filere.remove()) {
        ;

    } else {
        ;

    }
    // 复制文件
    if (QFile::copy(sourceFilePathre, targetFilePathre)) {
        ;
    } else {
        ;
    }

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


// 测试输出CustomQTreeWidget左侧树的所有项目的函数实现
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
    QString rootPath = currentPath;
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

// 创建新的工区 链接到创建树视图的函数
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
    }
    // else {
    //     return;
    // }
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
    // int eStarttime = Starttime.right(1).toInt();
    // int eEndtime = Endtime.right(1).toInt();
    // int result = eStarttime - eEndtime;
    // if (eStarttime+1 == 10){
    //     eStarttime = 0;
    // }else{
    //     eStarttime = eStarttime+1;
    // }
    // if (result % 2 == 0){
    //     Starttime.chop(1);         // 移除最后一个字符
    //     Starttime.append(QString::number(eStarttime));
    // }
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "resistics\n";
        out << processlineworking+"\n";
        out << airTS3+"\n";
        out << groundTS3+"\n";
        out << airTbl+"\n";
        out << groundTbl+"\n";
        out << LINfile+"\n";
        out << "line"+linname+"\n";
        out << yearandday+"-"+Starttime+"\n";
        out << yearandday+"-"+Endtime+"\n";
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



// 函数：复制文件 弃用
bool copyFile(const QString &sourceFilePath, const QString &destFilePath) {
    QFile file(sourceFilePath);
    if (!file.exists()) {
        qDebug() << "Source file does not exist:" << sourceFilePath;
        return false;
    }

    QDir destDir(QFileInfo(destFilePath).absolutePath());
    if (!destDir.exists()) {
        if (!destDir.mkpath(".")) {
            qDebug() << "Failed to create destination directory:" << destDir.absolutePath();
            return false;
        }
    }

    if (file.copy(destFilePath)) {
        // 更新目标文件的时间为当前时间

        QFile destFile(destFilePath);
        if (destFile.open(QIODevice::ReadWrite)) { // 确保文件可以被操作
            qDebug() << "File ReadOnly:";
            destFile.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
            destFile.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileAccessTime);
            destFile.close();
        }
        // qDebug() << "File copied:" << sourceFilePath << "->" << destFilePath;
        return true;
    } else {
        // qDebug() << "Failed to copy file:" << sourceFilePath;
        return false;
    }
}

// 函数：将源文件夹中的所有文件复制到目标文件夹下的同名子目录 弃用
void copyFilesToSameNameDir(const QString &sourceDirPath, const QString &destDirPath) {
    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist:" << sourceDirPath;
        return;
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for (const QString &fileName : files) {
        QString sourceFilePath = sourceDir.absoluteFilePath(fileName);

        // 创建目标目录的同名子文件夹
        QString destSubDirPath = QDir(destDirPath).absoluteFilePath(sourceDir.dirName());
        QString destFilePath = QDir(destSubDirPath).absoluteFilePath(fileName);

        copyFile(sourceFilePath, destFilePath);
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
        qDebug() << "removeRecursively" << Path+"/resistics";
    }
    // refresh.cdUp();
    // 检查 result 文件夹是否存在
    if (refresh.cd("result")) { // 进入 result 文件夹
        calculate = 0;
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
        //多次循环保证可以计算出结果 已注释
        calculate +=1;
        // if (calculate == 1){
        //     processlinereadfile=processlinewritefile;
        //     progressLineDialog =new QProgressDialog("计算中...", "取消", 0, 0, this);
        //     progressLineDialog->setWindowModality(Qt::WindowModal);
        //     progressLineDialog->setAutoClose(true);  // 完成后自动关闭
        //     progressLineDialog->setAutoReset(true);
        //     progressLineDialog->setCancelButton(nullptr);  // 不显示取消按钮
        //     progressLineDialog->setWindowTitle("倾子计算中...请稍等");
        //     progressLineDialog->show();
        //     if (!processlinereadfile.isEmpty()){
        //         // Create and start the process thread
        //         ProcessLine *processThread = new ProcessLine(this);
        //         connect(processThread, &ProcessLine::finished, this, &MainWindow::onProcessFinished);
        //         processThread->start();
        //     }
        //     return;
        // }
        qDebug() << "Result folder does not exist.";

        // 复制bins中的文件到界面 弃用
        QString targetFilePath = currentPath+ "/Tipper";
        QDir dircp(targetFilePath+"/line"+ linname);
        if (dircp.exists()) {
            dircp.removeRecursively();
        }
        QString sourceFilePath = QDir::currentPath() + "/bins/line"+ linname;
        copyFilesToSameNameDir(sourceFilePath,targetFilePath);
    }
    QDir dire(Path+"/result");

    if (dire.exists()) {
        dire.removeRecursively();
    }

    this->refresh_tree(Path);

    // this->refresh_view(Path,this);

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

// 计算两个数字的数量级
double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2) + std::pow(z2 - z1, 2));
}
// 必较数字数量级
bool areSameMagnitude(double a, double b, double tolerance = 1.0) {
    // 计算 a 和 b 的对数值
    double logA = std::log10(std::fabs(a)); // 对数值
    double logB = std::log10(std::fabs(b));

    // 计算两个对数值的差异
    return std::fabs(int(logA) - int(logB)) < tolerance; // 判断是否在容忍范围内
}

// 选择反演测线后创建反演结果文件夹，读取反演所需文件数据，设定反演模型相关参数,
// 写入反演模型文件.rho反演数据文件.data
// 开始反演，结果信号由其它函数处理
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
    double sumdistance = 0.0;
    for (int i=0;i< distance_two_points.size();i++){
        sumdistance+=distance_two_points[i];
    }
    qDebug() << "sumdistance" <<sumdistance ;
    int nxblocks =int(sumdistance/distance_two_points[0])*2 + 20, nyblocks= 56, nsameblocks = int(sumdistance/distance_two_points[0]), nysameblocks = 10;
    // nyblocks = nyblocks*2;
    if (nsameblocks > nxblocks*0.5) nsameblocks = int(nxblocks*0.5);
    if (int(nxblocks*0.5)!= nxblocks*0.5) nxblocks = nxblocks-1;
    if (int(nyblocks)!= nyblocks) nyblocks = nyblocks-1;
    // 数组初始化为均匀剖分部分
    QList<double> xblocks(nsameblocks, int(distance_two_points[0]*(0.5)));
    QList<double> yblocks(nysameblocks, int(10));

    double sumdistancex = 0.0;
    for (int i=0;i< xblocks.size();i++){
        sumdistancex+=xblocks[i];
    }
    qDebug() << "sumdistancex" <<sumdistancex  <<  nsameblocks;
    // for (double num : xblocks) {
    //     qDebug() << num ;
    // }
    bool breakbool =false;
    for (int i=0;i < int(nxblocks*0.5) - nsameblocks;i++){
        if (breakbool) break;
        xblocks.append(int(xblocks[xblocks.size()-1]+distance_two_points[0]/(int(nxblocks*0.5) - nsameblocks-i) + xblocks[xblocks.size()-2]) );
        // 直接增最大数量级
        if (areSameMagnitude(xblocks[xblocks.size()-1], xblocks[xblocks.size()-2])) {
            continue;
        } else {
            for (int j=0; j< int(nxblocks*0.5) - nsameblocks - i-1; j++){

                int magnitude =std::log10(std::fabs(xblocks[xblocks.size()-1]));
                // qDebug() << xblocks[xblocks.size()-1]<< magnitude ;
                double result = std::pow(10, magnitude);
                xblocks.append(int(xblocks[xblocks.size()-1]+ result));
                if (j == (int(nxblocks*0.5) - nsameblocks - i - 2)) breakbool = true;
            }

        }
    }
    qDebug() << xblocks.size();

    bool ybreakbool =false;
    double sumy = 0.0;
    for (int i=0;i < int(nyblocks) - nysameblocks;i++){
        if (ybreakbool) break;

        yblocks.append(int(yblocks[yblocks.size()-1]*1.1) );
        sumy+= yblocks[yblocks.size()-1];
        // qDebug() << yblocks[yblocks.size()-2];
        double temp = yblocks[yblocks.size()-2];
        // if (areSameMagnitude(yblocks[yblocks.size()-1], yblocks[yblocks.size()-2])) {
        if (sumy < 2000){
            continue;
        } else {
            for (int j=0; j< int(nyblocks)- nysameblocks - i ; j++){

                int magnitude =std::log10(std::fabs(temp));
                // qDebug() << temp<< magnitude ;

                double result = std::pow(10, magnitude);
                temp += result;
                yblocks.append(temp);
                if (j == (int(nyblocks) - nysameblocks - i - 1)) ybreakbool = true;
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

    qDebug() << xnewBlocks.size();
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

    if (int(nyblocks)!= nyblocks){
        int middleIndex = int(ynewBlocks.size() / 2);
        ynewBlocks.insert(middleIndex, ynewBlocks[ynewBlocks.size() / 2]);
    }


    nyblocks = int(nyblocks);
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

        out << QString::number(100)+  "\t" + QString::number(100) <<"\n";

        // for (int a= 0;a< nyblocks;a++){
        //     if (a < int (nyblocks*0.5)){
        //         out << QString::number(1)+  "\t" ;
        //     }else{
        //         out << QString::number(0)+  "\t" ;
        //         if (a == (nyblocks-1)){
        //             out << "\n" ;
        //         }
        //     }

        // }
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
        double sumall = 0.0;
        for (int a =0;a< distance_two_points.size(); a++){
            sumall += distance_two_points[a];
        }
        xsum=sumall;
        qDebug() << "xsum" << xsum;
        double sumx=0.0;
        for (int b =0;b < xoldblocks.size() ; b++){
            sumx += xoldblocks[b];
        }
        double sum = sumx - sumall/2;
        for (int i = 0;i < frequencies.size(); i++){
            QString iformattedString = QString("%1").arg(i+1, 3, 10, QChar('0'));


            // 累加从第0个元素到第i-1个元素
            if (i != 0) sum += distance_two_points[i-1];

            for (int j = 0 ;j< frequencies[0].size();j++){

                out << QString::number(frequencies[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ iformattedString+ "\t"+ "0.000"+ "\t"+ "0.000"+ "\t"+"0.000"+ "\t"+ QString::number(sum)+ "\t"+ "0.000" + "\t"+ "Ty" + "\t"+ QString::number(tipperReal[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ QString::number(tipperImag[i][frequencies[0].size()-j-1],'e', 6).replace('e', 'E') + "\t"+ QString::number( tipperReal[i][frequencies[0].size()-j-1] ,'e', 6).replace('e', 'E') + "\n";
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
    inverseprocessparameters << "-I" << "NLCG" <<currentPath + "/Inverse/"+linename+"/model.rho"  << currentPath + "/Inverse/"+linename+"/data.dat" << "1" << "1";
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
    QDir dir(Path);

    // 获取路径下所有文件的列表
    QStringList files = dir.entryList(QDir::Files);

    QStringList rhoFiles;
    QStringList resPrmFiles;
    QStringList rhoFileMap;  // 用于存储.rho文件及其对应的数字

    // 遍历所有文件
    for (const QString &fileName : files) {
        // 判断文件扩展名并分类
        if (fileName.endsWith(".rho", Qt::CaseInsensitive)) {


            rhoFileMap.append(fileName);  // 存储该文件及其数字


        } else if (fileName.endsWith(".res", Qt::CaseInsensitive) || fileName.endsWith(".prm", Qt::CaseInsensitive) || fileName.endsWith(".dat", Qt::CaseInsensitive) || fileName.endsWith(".log", Qt::CaseInsensitive)) {
            resPrmFiles.append(fileName);  // 添加到删除列表
        }
    }

    // 删除 .res 和 .prm 文件
    for (const QString &fileName : resPrmFiles) {
        QFile file(dir.filePath(fileName));
        if (file.remove()) {
            ;
        } else {
            qDebug() << "Failed to delete file:" << fileName;
        }
    }

    // 对于 .rho 文件，保留最大数字的文件，删除其他文件
    if (!rhoFileMap.isEmpty()) {

        int i =0;
        for (const QString &fileName : rhoFileMap) {
            if (i != rhoFileMap.size()-1) {
                QFile file(dir.filePath(fileName));
                if (file.remove()) {
                    ;
                } else {
                    qDebug() << "Failed to delete file:" << fileName;
                }
            }else{
                QFile file(dir.filePath(fileName));
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QMessageBox::warning(this, "错误", "无法打开文件");
                    return;
                }
                qDebug()<< "douclick clicked file"<<dir.filePath(fileName);


                QTextStream in(&file);
                QString line = in.readLine();
                QStringList tokens =  line.split(' ', Qt::SkipEmptyParts);
                qDebug()<<tokens<<dir.filePath(fileName);

                int xblock = tokens[0].toInt();
                int yblock = tokens[1].toInt();

                qDebug() << "xblock: " << xblock << ", yblock: " << yblock ;
                xblocks.clear();
                yblocks.clear();
                data.clear();
                // 读取 xblocks
                while (!in.atEnd()) {
                    line = in.readLine();
                    QStringList numbers = line.split(' ', Qt::SkipEmptyParts);
                    for (const QString& num : numbers) {
                        xblocks.push_back(num.toDouble());
                        if (xblocks.size() == xblock) {
                            break;
                        }
                    }
                    if (xblocks.size() == xblock) break;
                }

                // 读取 yblocks
                while (!in.atEnd()) {
                    line = in.readLine();
                    QStringList numbers = line.split(' ', Qt::SkipEmptyParts);
                    for (const QString& num : numbers) {
                        yblocks.push_back(num.toDouble());
                        if (yblocks.size() == yblock) {
                            break;
                        }
                    }
                    if (yblocks.size() == yblock) break;
                }

                line = in.readLine();
                // 读取 data
                std::vector<double> temp;
                while (!in.atEnd()) {
                    line = in.readLine();
                    QStringList numbers = line.split(' ', Qt::SkipEmptyParts);
                    for (const QString& num : numbers) {
                        temp.push_back(num.toDouble());
                        if (temp.size() == xblock) {
                            data.push_back(temp);
                            temp.clear();
                        }
                    }
                }

                // 读取反演结果数据，截取保留中间部分的数据并写入文件中
                // 这些数据可以用来绘制图形
                qDebug()<< data[0];
                QList<double> x_coords ;  // 初始 0
                QList<double> x_coordsfinal ;
                QList<double> y_coords;  // 初始 0

                // 对 xblocks 和 yblocks 进行累加，生成 x_coords 和 y_coords
                double cumulative_x = 0;
                for (double x : xblocks) {
                    cumulative_x += x;
                    x_coords.push_back(cumulative_x);
                }

                double cumulative_y = 0;
                for (double y : yblocks) {
                    cumulative_y += y;
                    if (cumulative_y < 2000){
                        y_coords.push_back(cumulative_y);
                    }

                }

                double half_last_x =0.0;
                for (int a =0;a< xblocks.size() ;a++){

                    half_last_x+=xblocks[a];
                    // qDebug()<< x_coords[a];
                }

                for (int a =0;a< y_coords.size() ;a++){
                    qDebug()<< y_coords[a];
                }

                // 调整 x 坐标
                half_last_x = half_last_x / 2.0;

                for (int i = 0; i < x_coords.size(); ++i) {
                    x_coords[i] -= half_last_x;
                    qDebug() << x_coords[i];
                }

                int flagleft = 0;
                int flagright = 0;
                for (int i = 0; i < x_coords.size(); ++i) {
                    if (x_coords[i] > -(xsum/2) && x_coords[i] < (xsum/2)){
                        x_coordsfinal.append(x_coords[i]);
                        if (flagleft == 0) flagleft = i+1;
                        flagright = i-1;

                    }
                    // if (x_coords[i] < (xsum/2 + xblocks[int(xblocks.size()/2)])){
                    //     x_coordsfinal.append(x_coords[i]);
                    //     flagright = i;
                    // }
                }

                int deletexnum = x_coords.size() -x_coordsfinal.size();
                std::vector<std::vector<double>> datafinal(y_coords.size(), std::vector<double>(x_coordsfinal.size(), 0));
                qDebug() << x_coordsfinal.size() << y_coords.size() << datafinal[0].size() << datafinal.size() << flagleft << flagright << xsum;
                for(int i =0;i< data.size();i++){
                    if(i < y_coords.size()){
                        for (int j =0;j< x_coordsfinal.size();j++){
                            datafinal[i][j] = data[i][j+flagleft];
                            // qDebug() << "j+flagleft"<< j+flagleft;
                        }
                    }
                }
                QFile datfilefordir(dir.filePath(fileName));
                QFileInfo fileInfo(datfilefordir.fileName()); // 获取 QFileInfo 对象
                QString directory = fileInfo.absolutePath();
                QFile datfile(directory+"/data.rho");
                if (datfile.exists()) datfile.remove();
                if (datfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&datfile);
                    for (int i = 0; i < y_coords.size(); ++i) {
                        for(int j = 0; j < x_coordsfinal.size(); j++){
                            out << QString::number(x_coordsfinal[j]) + "\t" + QString::number(y_coords[i]) + "\t" + QString::number(datafinal[i][j]) + "\n";
                        }

                    }
                    datfile.close();

                }

                if (file.remove()) {
                    ;
                } else {
                    qDebug() << "Failed to delete file:" << fileName;
                }
            }
            i+=1;
        }
    }

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

// 打开反演选择测线的窗口，返回选择测线的信息，包含路径信息
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

