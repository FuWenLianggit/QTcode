#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>

class ProgressDialog : public QProgressDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    void setCustomStyle();

signals:
    void progressUpdated(int value);

public slots:
    void updateProgress(int value);

private:
    void setupStyle();
};

#endif // PROGRESSDIALOG_H
