#ifndef DEVICEWINDOW_H
#define DEVICEWINDOW_H

#include <QDialog>

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
    void LoadDevices();

private:
    Ui::DeviceWindow *ui;
};

#endif // DEVICEWINDOW_H
