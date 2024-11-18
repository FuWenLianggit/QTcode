#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QHBoxLayout>


class ImportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = nullptr);
    QStringList getSelectedFilePath() const;

private slots:
    void onSelectButtonClicked();

private:
    QListWidget *fileTypeList;
    QString selectedFilePath;
    QPushButton *selectButton;
    QStringList fileandtype;
};

#endif // IMPORTDIALOG_H
