#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QDialog>
#include <QTreeWidget>

class MainWindow;  // 前向声明

class FileDialog : public QDialog {
    Q_OBJECT

public:
    explicit FileDialog(QWidget *parent = nullptr);

    QString getSelectedFilePath() const;

    QString selectedFilePath;
private slots:
    void itemDoubleClicked(QTreeWidgetItem *item, int column);
    void new_view();
    void buildTree(const QString &path, QTreeWidgetItem *parentItem);


private:
    QTreeWidget *treeWidget;
    MainWindow *mainWindow;  // 存储 MainWindow 的指针

};

#endif // FILEDIALOG_H
