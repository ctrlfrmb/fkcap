#ifndef NETWORKASSISTWINDOW_H
#define NETWORKASSISTWINDOW_H

#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include "def.h"
#include "networkhelper.h"
#include "common/basecomm.h"

namespace Ui {
class NetworkAssistWindow;
}

class NetworkAssistWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkAssistWindow(QWidget *parent = nullptr);
    ~NetworkAssistWindow();

    void setWindow(bool isService);
    void set(figkey::PacketInfo packet);

public slots:
    void onDataReceived(const QByteArray& data);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

    void on_buttonSend_clicked();

    void on_buttonStopSend_clicked();

    void on_comboBox_currentIndexChanged(int index);

private:
    void closeComm();
    void initWindow();
    void exitWindow();
    void setProtocol(uint8_t protocol, QComboBox* comboBox);
    void setClientIP(QComboBox* comboBox, const figkey::PacketInfo& packet);
    void setServerIP(QComboBox* comboBox, const figkey::PacketInfo& packet);
    void setServerPort(QComboBox* comboBox, const figkey::PacketInfo& packet);
    QComboBox* getSettingComboBox(const QString& label);
    QString getSettingItemValue(const QString& label) const;
    bool sendMessage(int row);
    bool sendMessages(const QList<int>& sendList);
    bool sendAndReceiveMessage();

private:
    Ui::NetworkAssistWindow *ui;
    NetworkHelper *helper { nullptr };
    BaseComm *comm { nullptr };

    bool isServer { false };
};

#endif // NETWORKASSISTWINDOW_H
