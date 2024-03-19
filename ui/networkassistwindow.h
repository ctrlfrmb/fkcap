#ifndef NETWORKASSISTWINDOW_H
#define NETWORKASSISTWINDOW_H

#include <QDialog>
#include <QCloseEvent>
#include <QComboBox>
#include "def.h"
#include "networkhelper.h"
#include "common/basecomm.h"
#include "doip/doiphelper.h"

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
    bool saveConfigToFile();
    static bool isServerByLoadFile(const QString& fileName, bool& hasServer);
    bool loadConfigFromFile(const QString& fileName);
    void set(figkey::PacketInfo packet);
    void setSimulation(figkey::PacketInfo packet);
    void setMessageType(int type);
    void addRow(const figkey::PacketInfo& packet);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

    void on_buttonSend_clicked();

    void on_buttonStopSend_clicked();

    void on_comboBox_currentIndexChanged(int index);

    bool sendMessage(int row);

    void onDataReceived(const QByteArray& data);

    void on_buttonDoIPSet_clicked();

    void on_buttonVehicleRequst_clicked();

private:
    void closeComm();
    void isSaveFile();
    void initWindow();
    void exitWindow();
    void autoTest();

    void setProtocol(uint8_t protocol, QComboBox* comboBox);
    bool isLocalIP(const std::string& ip);
    void setClientIP(QComboBox* comboBox, const figkey::PacketInfo& packet);
    void setServerIP(QComboBox* comboBox, const figkey::PacketInfo& packet);
    void setServerPort(QComboBox* comboBox, const figkey::PacketInfo& packet);
    QComboBox* getSettingComboBox(const QString& label);
    QString getSettingItemValue(const QString& label) const;

    bool sendMessages(const QList<int>& sendList);
    void sendNextMessage(const QList<int>& sendList, int currentIndex);
    bool sendAndReceiveMessage();

private:
    Ui::NetworkAssistWindow *ui;
    NetworkHelper *helper { nullptr };
    BaseComm *comm { nullptr };
    DoIPHelper *doip { nullptr };

    bool isServer { false };
    bool canSaveFile { false };
};

#endif // NETWORKASSISTWINDOW_H
