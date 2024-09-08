#include "progressdialog.h"
#include <QApplication>
#include <QStyle>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QProgressDialog("Copying files...", "Cancel", 0, 100, parent) {
    setCustomStyle();
    setWindowModality(Qt::WindowModal);
    setMinimumDuration(0);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAutoReset(false);
    setAutoClose(false);
}

void ProgressDialog::setCustomStyle() {
    // Custom styles for the progress dialog
    setStyleSheet("QProgressDialog {"
                  "border: 2px solid #2a2a2a;"
                  "border-radius: 10px;"
                  "background-color: #f5f5f5;"
                  "padding: 10px;"
                  "}"
                  "QProgressBar {"
                  "border: 1px solid #2a2a2a;"
                  "border-radius: 5px;"
                  "background-color: #e0e0e0;"
                  "text-align: center;"
                  "}"
                  "QProgressBar::chunk {"
                  "background-color: #4caf50;"
                  "width: 20px;"
                  "}"
                  "QPushButton {"
                  "background-color: #4caf50;"
                  "color: white;"
                  "border: none;"
                  "border-radius: 5px;"
                  "padding: 5px;"
                  "}");

    // Set additional style to the progress dialog to make it more beautiful
    setupStyle();
}

void ProgressDialog::setupStyle() {
    // Example of further customization, if needed
}

void ProgressDialog::updateProgress(int value) {
    setValue(value);
}
