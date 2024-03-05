#ifndef DOIPCLIENTWINDOW_H
#define DOIPCLIENTWINDOW_H

#include "doip/doipclient.h"
#include <QDialog>
#include <QCloseEvent>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <atomic>
#include <QMutex>

namespace Ui {
class DoIPClientWindow;
}

class DoIPClientWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DoIPClientWindow(QWidget *parent = 0);
    ~DoIPClientWindow();

    void receiveMessage(std::vector<uint8_t> data);

private slots:

    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

    void slotSendData(int row, QLineEdit *lineEdit, QCheckBox *checkBox, QSpinBox *intervalSpin);

protected:
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void initSetting();
    void initTableSend();
    void initTableReceive();
    void initWindow();

    Ui::DoIPClientWindow *ui;
    std::atomic<bool> isConnected{false};
    figkey::DoIPClient doip;
    QMutex mutexTimers;
    QMap<int, int> timerIDs;

};

#endif // DOIPCLIENTWINDOW_H
