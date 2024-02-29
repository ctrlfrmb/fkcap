#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sqlite.h"
#include "packeinfo.h"
#include <QCloseEvent>
#include <QStandardItemModel>

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

public slots:
    void updateUI();  // 当数据变化时更新 UI

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:

    void on_actionStop_triggered();

    void on_actionStart_triggered();

    void on_actionPause_triggered();

private:
    void initTableView();
    void initTreeView();
    void initWindow();
    void updateTreeView();

private:
    Ui::MainWindow *ui;
    SqliteCom db;
    PacketInfoModel *pim;
    QStandardItemModel *tvm;
    QTimer *m_timerUpdateUI;  // 将定时器定义为类的成员变量
};

#endif // MAINWINDOW_H
