#ifndef DEVICEWINDOW_H
#define DEVICEWINDOW_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class DeviceWindow;
}

class DeviceWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceWindow(QWidget *parent = 0);
    ~DeviceWindow();

private slots:
    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_pushButton_clicked();

private:
    void initWindow();
    void exitWindow();

private:
    Ui::DeviceWindow *ui;
    QStandardItemModel *model;
};

#endif // DEVICEWINDOW_H
