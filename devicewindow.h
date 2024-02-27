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

    const QString& getNetworkName() const { return networkName; }

private slots:
    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_pushButton_clicked();

private:
    void LoadDevices();
    void ExitWindow();

private:
    Ui::DeviceWindow *ui;
    QString networkName;
};

#endif // DEVICEWINDOW_H
