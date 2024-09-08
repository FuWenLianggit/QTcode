#include "ImportDialog.h"

ImportDialog::ImportDialog(QWidget *parent) : QDialog(parent), selectedFilePath("") {
    this->setWindowTitle("导入文件");
    this->setFixedSize(400, 300);  // 设置固定大小

    // 设置布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 设置主布局的内边距，使 QGroupBox 上方有一些空间
    mainLayout->setContentsMargins(10, 20, 10, 10);  // 设置上边距为20，其他边距为10

    // 创建文件类型选择分组
    QGroupBox *fileTypeGroupBox = new QGroupBox("选择导入的文件类型", this);
    QVBoxLayout *fileTypeLayout = new QVBoxLayout(fileTypeGroupBox);
    fileTypeLayout->setContentsMargins(10, 20, 10, 10);  // 设置上边距为20，其他边距为10
    // 创建文件类型列表 "airTBL" << "airTS3" << "groundTBL" << "groundTS3" << "LIN";"AirTBL" << "AirTS3" << "GroundTBL" << "GroundTS3" << "LIN";
    fileTypeList = new QListWidget(this);
    fileTypeList->addItem("AirTBL");
    fileTypeList->addItem("AirTS3");
    fileTypeList->addItem("GroundTBL");
    fileTypeList->addItem("GroundTS3");
    fileTypeList->addItem("LIN");
    fileTypeLayout->addWidget(fileTypeList);

    // 添加分组到主布局
    mainLayout->addWidget(fileTypeGroupBox);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignRight);

    // 创建选择按钮
    selectButton = new QPushButton("选择", this);
    selectButton->setFixedSize(80, 30);  // 设置按钮大小
    selectButton->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; border-radius: 5px; }");
    buttonLayout->addWidget(selectButton);

    mainLayout->addLayout(buttonLayout);

    // 连接按钮点击事件
    connect(selectButton, &QPushButton::clicked, this, &ImportDialog::onSelectButtonClicked);

    // 设置整体样式
    this->setStyleSheet("QDialog { background-color: #f0f0f0; } QGroupBox { font-weight: bold; border: 1px solid #d3d3d3; border-radius: 5px; margin-top: 10px; }");
}

QStringList ImportDialog::getSelectedFilePath() const {
    return fileandtype;
}

void ImportDialog::onSelectButtonClicked() {
    if (fileTypeList->currentItem() == nullptr) {
        QMessageBox::warning(this, "警告", "请选择一个文件类型。");
        return;
    }

    QString filter;
    QString fileType = fileTypeList->currentItem()->text();

    // 根据选中的文件类型设置文件过滤条件
    if (fileType == "AirTS3" || fileType == "GroundTS3") {
        filter = "TS3 files (*TS3.txt)";
    } else if (fileType == "AirTBL" || fileType == "GroundTBL") {
        filter = "Text files (*.txt);;Not TS3 files ([!T][!S][!3].txt)";
    } else if (fileType == "LIN") {
        filter = "LIN files (*.LIN.txt)";
    }

    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", filter);
    if (!filePath.isEmpty()) {
        selectedFilePath = filePath;
        fileandtype <<  fileType<<filePath;
        accept();  // 关闭对话框并返回接受状态
    }
}
