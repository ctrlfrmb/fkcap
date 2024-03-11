#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sqlite.h"
#include "packeinfo.h"
#include <QMainWindow>
#include <QCloseEvent>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool isStart = true, QWidget *parent = 0);
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

    void on_actionFilter_triggered();

    void on_actionFilter_Clear_triggered();

    void on_actionDoIP_Client_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_tableView_clicked(const QModelIndex &index);

    void on_actionNetwork_Assist_triggered();

private:
    void initTableView();
    void initTreeView();
    void initWindow(bool isStart);
    void exitWindow();
    void updateTreeView(const figkey::PacketInfo& packet);
    void pauseCapture();
    void reumeCapture();

private:
    Ui::MainWindow *ui;

    QMutex mutexPacket;
    uint64_t packetCounter{ 0 };
    SqliteCom db;
    PacketInfoModel *pim{ nullptr};
    QStandardItemModel *tvm{ nullptr};
    QTimer *timerUpdateUI{ nullptr};  // 将定时器定义为类的成员变量
    bool scrollBarAtBottom{ true };
    bool userHasScrolled{ false };
};

#endif // MAINWINDOW_H
