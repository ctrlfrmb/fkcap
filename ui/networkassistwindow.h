#ifndef NETWORKASSISTWINDOW_H
#define NETWORKASSISTWINDOW_H

#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include "def.h"
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
    void syncPort(int index);
    void onDataReceived(const QByteArray& data);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

    void on_buttonSend_clicked();

    void on_buttonSendReceive_clicked();

    void on_buttonSendPeriod_clicked();

    void on_buttonSendSequence_clicked();

    void on_buttonStopSend_clicked();

private:
    void addSettingItem(bool isEdit, const QString& label, const QStringList& options);
    void initListSetting();
    void initTableSend();
    void initTableReceive();
    void initWindow();
    void exitWindow();
    void setProtocol(uint8_t protocol, QComboBox* comboBox);
    void setClientIP(QComboBox* comboBox, const figkey::PacketInfo& packet);
    void setServerIP(QComboBox* comboBox, const figkey::PacketInfo& packet);
    void setServerPort(QComboBox* comboBox, const figkey::PacketInfo& packet);
    QComboBox* getSettingComboBox(const QString& label);
    QString getSettingItemValue(const QString& label) const;
    void setSendButton(bool enable);
    bool getSendChecked(int row);
    void setSendChecked(int row, bool check);
    QByteArray getSendData(int row);
    bool sendMessage(int row, const QByteArray& data);
    void updateSendChecked(const QString& timeStamp, const QString& dataString);
    bool validateSendReceiveSequence(QList<QMap<QString, QVariant>>& list);

private:
    Ui::NetworkAssistWindow *ui;
    QList<QComboBox*> comboBoxes;
    bool isServer { false };
    bool isASCII { false };
    BaseComm* comm{ nullptr };
    QMap<int, QByteArray> recvDataMap;
    bool stopSending = true;
};

#endif // NETWORKASSISTWINDOW_H
