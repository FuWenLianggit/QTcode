#include "TipperDialog.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileInfo>
#include <QDebug>
#include <QSpinBox>

// 倾子计算参数选择界面实现 将文件路径和参数返回到主界面的tipper中使用
TipperDialog::TipperDialog(MainWindow *parent)
    : QDialog(parent), currentStep(0), mainWindow(parent)
{
    setupUi();
    checkAndAdjustList1(); // 初始化时检查并调整 list1
}

void TipperDialog::setupUi()
{
    this->setWindowTitle("选择文件");
    this->setFixedSize(600, 400);

    // 创建UI组件
    instructionLabel = new QLabel("请选择倾子计算所需文件", this);
    QFont labelFont;
    labelFont.setPointSize(12);
    instructionLabel->setFont(labelFont);
    instructionLabel->setAlignment(Qt::AlignCenter);

    list1 = new QListWidget(this);
    list2 = new QListWidget(this);
    nextButton = new QPushButton("下一步", this);
    nextButton->setEnabled(false);

    selectedFiles <<  "airTBL file not found" << "airTS3 file not found" << "groundTBL file not found" << "groundTS3 file not found" << "LIN file not found"<<"LINlines not found" <<"Splitting time not found";
    // 添加选项到 list1
    // {"Tipper","AirTS3", "GroundTS3", "AirTBL","GroundTBL","LIN"};
    options << "空中TBL" << "空中TS3" << "地面TBL" << "地面TS3" << "飞行轨迹"<< "LINlines" << "分割时间";
    for (const QString &option : options) {
        QListWidgetItem *item = new QListWidgetItem(option, list1);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled); // 初始时禁用所有选项
        item->setForeground(QBrush(Qt::gray)); // 灰色显示
    }

    // 设置列表1的样式
    list1->setFixedWidth(200);
    list1->setStyleSheet("QListWidget { background-color: #f0f0f0; } QListWidget::item:selected { background-color: #a0a0ff; }");

    // 设置列表2的样式
    list2->setStyleSheet("QListWidget { background-color: #ffffff; } QListWidget::item:selected { background-color: #a0a0ff; }");

    // 布局设置
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(instructionLabel);
    leftLayout->addWidget(list1);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(list2);

    QHBoxLayout *listsLayout = new QHBoxLayout();
    listsLayout->addLayout(leftLayout);
    listsLayout->addLayout(rightLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(nextButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(listsLayout);
    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(list1, &QListWidget::itemSelectionChanged, this, &TipperDialog::onList1ItemSelected);
    connect(list2, &QListWidget::itemSelectionChanged, this, &TipperDialog::onList2ItemSelected);
    connect(nextButton, &QPushButton::clicked, this, &TipperDialog::onNextButtonClicked);
}

void TipperDialog::checkAndAdjustList1()
{
    if (!mainWindow) return;

    QTreeWidget *customTreeWidget = mainWindow->findChild<QTreeWidget*>();
    if (!customTreeWidget) return;

    bool foundFirstMatchingOption = false;
    QListWidgetItem *first = list1->item(0);
    bool iffirst=true;
    std::function<void(QTreeWidgetItem*)> traverseItems = [&](QTreeWidgetItem* item) {
        QString itemText = item->text(0);

        // 检查是否符合条件：存在于 options 中且至少有一个子项
        if (options.contains(itemText) && item->childCount() > 0 ) {
            for (int i = 0; i < list1->count(); ++i) {
                QListWidgetItem *listItem = list1->item(i);
                if(iffirst){
                    first = listItem;
                    iffirst=false;
                }
                qDebug() <<listItem->text()<<  list1->item(i) << i;
                if (listItem->text() == itemText) {
                    validOptions << listItem->text();  // 添加到有效选项中

                    listItem->setFlags(listItem->flags() | Qt::ItemIsEnabled);
                    listItem->setForeground(QBrush(Qt::black)); // 黑色显示

                    if (!foundFirstMatchingOption) {
                        list1->setCurrentItem(first);  // 将第一个匹配项设为当前选项
                        QListWidgetItem *list2firstItem = list2->item(0);
                        list2->setCurrentItem(list2firstItem);
                        foundFirstMatchingOption = true;
                    }
                    break;
                }

            }
        }

        for (int i = 0; i < item->childCount(); ++i) {
            traverseItems(item->child(i));
        }
    };

    for (int i = 0; i < customTreeWidget->topLevelItemCount(); ++i) {
        traverseItems(customTreeWidget->topLevelItem(i));
    }

    for (int i = 0; i < list1->count(); ++i) {
        QListWidgetItem *listItem = list1->item(i);
        if (list1->item(i)->text() == "分割时间"){
            validOptions << listItem->text();
            listItem->setFlags(listItem->flags() | Qt::ItemIsEnabled);
            listItem->setForeground(QBrush(Qt::black)); // 黑色显示
        }
    }

    qDebug() <<validOptions;
    // 更新按钮状态
    updateNextButtonState();
}

void TipperDialog::updateNextButtonState()
{
    // return;
    nextButton->setEnabled(list2->currentItem() != nullptr);
}

void TipperDialog::onList1ItemSelected()
{
    QList<QListWidgetItem*> selectedItems = list1->selectedItems();
    if (selectedItems.isEmpty()) {
        list2->clear();
        nextButton->setEnabled(false);
        return;
    }

    QListWidgetItem *selectedItem = selectedItems.first();
    QString selectedOption = selectedItem->text();

    if (validOptions.contains(selectedOption)) {
        populateList2();
        nextButton->setEnabled(!list2->selectedItems().isEmpty());
    } else {
        list2->clear();
        nextButton->setEnabled(false);
    }
}

void TipperDialog::populateList2()
{
    list2->clear();

    QString selectedType = list1->currentItem()->text();

    if (!mainWindow) return;

    QTreeWidget *customTreeWidget = mainWindow->findChild<QTreeWidget*>();
    if (!customTreeWidget) return;

    std::function<void(QTreeWidgetItem*)> traverseItems = [&](QTreeWidgetItem* item) {
        // qDebug() << "traverseItems " << item->text(0) << "selectedType "<< selectedType;
        if (item->text(0) == selectedType) {
            for (int j = 0; j < item->childCount(); ++j) {
                QTreeWidgetItem *child = item->child(j);
                QListWidgetItem *list2Item = new QListWidgetItem(child->text(0), list2);
                list2Item->setData(Qt::UserRole, child->data(0, Qt::UserRole));
            }
        }

        for (int i = 0; i < item->childCount(); ++i) {
            traverseItems(item->child(i));
        }
    };

    for (int i = 0; i < customTreeWidget->topLevelItemCount(); ++i) {
        traverseItems(customTreeWidget->topLevelItem(i));
    }
    if (selectedType == "分割时间") {
        // 创建 QSpinBox
        QSpinBox* spinBox = new QSpinBox(list2);
        spinBox->setValue(20); // 设置默认值为40
        spinBox->setMinimum(0); // 设置最小值（可以根据需要调整）
        spinBox->setMaximum(100); // 设置最大值（可以根据需要调整）

        // 创建 QListWidgetItem，并将 QSpinBox 添加到其中
        QListWidgetItem* spinBoxItem = new QListWidgetItem(list2);
        list2->addItem(spinBoxItem);
        list2->setItemWidget(spinBoxItem, spinBox);
    }
}

void TipperDialog::onList2ItemSelected()
{

    updateNextButtonState();

}

void TipperDialog::onNextButtonClicked()
{
    QListWidgetItem *selectedItem = list2->currentItem();
    if (!selectedItem) return;
    QString value;
    if (selectedItem) {
        // 获取当前条目的小部件
        QWidget* widget = list2->itemWidget(selectedItem);

        // 检查小部件是否是 QSpinBox
        QSpinBox* spinBox = dynamic_cast<QSpinBox*>(widget);
        if (spinBox) {
            // 获取 QSpinBox 的值
            int valueget = spinBox->value();
            value = QString::number(valueget);
            qDebug() << "Selected QSpinBox value:" << value;
        } else {
            qDebug() << "Selected item is not a QSpinBox.";
        }
    }
    QString filePath = selectedItem->data(Qt::UserRole).toString();
    int optionsmem=0;
    for (QString option : options){
        if (option == list1->currentItem()->text()){
            selectedFiles[optionsmem]=filePath;
            if (list1->currentItem()->text() == "LINlines"){
                selectedFiles[optionsmem]=selectedItem->text();
            }
        }
        if (option =="分割时间" && value != ""){
            selectedFiles[optionsmem]=value;
        }
        optionsmem++;
    }
    qDebug() << "selectedFiles" << selectedFiles;
    qDebug() << "validOptions" <<validOptions;
    // selectedFiles << filePath;
    QListWidgetItem *listitem= list1->currentItem();
    QString listitemstr = listitem->text();
    list1->currentItem()->setFlags(list1->currentItem()->flags() & ~Qt::ItemIsEnabled);
    list1->currentItem()->setForeground(QBrush(Qt::gray));

    currentStep++;
    QList<QListWidgetItem*> items = list1->findItems("*", Qt::MatchWildcard);
    qDebug() << items.size();
    if (currentStep < options.size()) {
        for (int i = 0; i < items.size(); ++i) {
            QListWidgetItem *item = items[i];
            if (i+1 == items.size()) i=-1;
            QListWidgetItem *nextItem = items[i+1];
            qDebug() <<listitemstr << nextItem->text();
            if (listitem->text() == item->text()){
                list2->clear();
                list1->setCurrentItem(nextItem);
                QListWidgetItem *list2firstItem = list2->item(0);
                list2->setCurrentItem(list2firstItem);
                break;
            }


        }

        // QListWidgetItem *nextItem = list1->item(currentStep);
        // if (validOptions.contains(nextItem->text())) {
        //     nextItem->setFlags(nextItem->flags() | Qt::ItemIsEnabled);
        //     nextItem->setForeground(QBrush(Qt::black));
        //     // list1->setCurrentRow(currentStep);
        //     list2->clear();
        //     nextButton->setEnabled(false);
        // }

        // QString currentoption=validOptions[currentStep-1];
        // bool foundCurrentOption = false;

        // for (int i = 0; i < items.size(); ++i) {
        //     QListWidgetItem *item = items[i];

        //     if (item->text() == currentoption) {
        //         foundCurrentOption = true;
        //         continue; // Move to the next item
        //     }

        //     if (foundCurrentOption) {
        //         qDebug()<<" item->text() =="<<   item->text() << "validOptions[currentStep+1]"<< validOptions[currentStep];
        //         if (item->text() == validOptions[currentStep]) {
        //             qDebug() << "Found next valid option:" << validOptions[currentStep];
        //             // Perform any action needed with the item
        //             // For example, select this item
        //             list1->setCurrentItem(item);
        //             break; // Exit loop if the next valid option is found
        //         } else {

        //             qDebug() << "Next valid option not found.";
        //             // break;
        //         }
        //     }
        // }
    } else {
        nextButton->setText("确定");
        nextButton->setEnabled(true);
        disconnect(nextButton, &QPushButton::clicked, this, &TipperDialog::onNextButtonClicked);
        connect(nextButton, &QPushButton::clicked, this, &TipperDialog::accept);
    }
}

QStringList TipperDialog::getSelectedFiles() const
{
    return selectedFiles;
}
