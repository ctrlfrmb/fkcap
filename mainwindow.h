#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sqlite.h"
#include "packeinfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void processPacket(figkey::PacketInfo packetInfo);

private:
    void LoadConfigFile();

private:
    Ui::MainWindow *ui;
    SqliteCom db;
    PacketInfoModel *pim;
};

#endif // MAINWINDOW_H
