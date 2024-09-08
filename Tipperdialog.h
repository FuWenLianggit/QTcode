#ifndef TIPPERDIALOG_H
#define TIPPERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QStringList>
#include <QTreeWidgetItem>

class MainWindow;

class TipperDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TipperDialog(MainWindow *parent = nullptr);
    QStringList getSelectedFiles() const;

private slots:
    void onList1ItemSelected();
    void onList2ItemSelected();
    void onNextButtonClicked();

private:
    void setupUi();
    void populateList2();
    void checkAndAdjustList1();
    void updateNextButtonState();

    QListWidget *list1;
    QListWidget *list2;
    QPushButton *nextButton;
    QLabel *instructionLabel;
    QStringList options;
    QStringList selectedFiles;
    QStringList validOptions;  // 用于存储可选的选项（首选和备选）
    int currentStep;
    MainWindow *mainWindow;
};

#endif // TIPPERDIALOG_H
